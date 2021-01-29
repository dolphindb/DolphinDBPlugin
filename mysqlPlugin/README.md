# Plugin for MySQL

利用MySQL的Trigger+UDF实时同步MySQL中的改动到DolphinDB内存表中。

## 1. linux下编译安装

### 1.1 相关环境

+ MySQL 8.0

+ [DolphinDB C++ API](https://github.com/dolphindb/api-cplusplus)及其环境

	API中的用于64位linux的动态库及头文件已包含在本文件夹bin和include下，如需其他版本可自行下载。

### 1.2 编译

#### 1.2.1 使用预编译库

在bin/linux_x64文件夹下有预先编译的MySQL插件libdolSync.so

#### 1.2.2 自行编译

使用cmake编译，依赖库包括openssl1.0.2，uuid，mysql

```shell
mkdir build
cd build
cmake ..
make
```

编译后目录下会产生libdolSync.so文件

### 1.3 安装

将上述库文件libdolSync.so移动到mysql的插件库文件夹(默认为 /usr/lib/mysql/plugin/ )中，
使用如下sql语句创建相关自定义函数。

```sql
create function dolInit returns integer soname 'libdolSync.so';
create function dolInsert returns integer soname 'libdolSync.so';
create function dolDelete returns integer soname 'libdolSync.so';
create function dolUpdate returns integer soname 'libdolSync.so';
```

>如果创建函数时mysql找不到动态库，可将缺少的库移动到系统默认库路径(ubuntu中为 /usr/lib/x86_64-linux-gnu/ )下。

## 2. 用户接口说明

1. dolInit(ip,port,userName,pwd)
    + ip：dolphindb服务器地址
    + port：服务器端口
    + userName：用户名
    + pwd：密码

    通过参数连接dolphindb服务器，并建立队列。
    当有同步需求时，都需要先执行该函数，启动后台线程。
2. dolInsert(tableName,colNames,newColValues)
    + tableName：dolphindb中被同步的表名
    + colNames：被同步表的列名
    + newColValues：新加入行的值

    将MySQL中新增的行同步到dolphindb

3. dolDelete(tableName,keyNames,oldColValues)
    + tableName：dolphindb中被同步的表名
    + keyNames：被删除行的主键在dolphindb中的列名
    + oldColValues：被删除行的主键的值

    在dolphindb中同步删除MySQL中被删除的行

4. dolUpdate(tableName,keyCnt,keyNames,oldColValues,colNames,newColValues)
    + tableName：dolphindb中被同步的表名
    + keyCnt：主键的个数
    + keyNames：被更新行的主键在dolphindb中的列名
    + oldColValues：更新前主键的值
    + colNames：被同步表的列名
    + newColValues：更新后列名对应的值

    在dolphindb中同步更新MySQL中更新的行

## 3. 实例

下面是一个MySQL表同步到DolphinDB的例子。

用如下sql语句在MySQL中demo数据库中创建表myTable，

```sql
CREATE TABLE `demo`.`myTable` (
  `int` INT NOT NULL,
  `str` VARCHAR(45) NULL,
  `double` DOUBLE NULL,
  PRIMARY KEY (`int`));
```
用如下语句在DolphinDB中创建内存表t并共享为tglobal，

```sql
t=table(100:0,`dint`dstr`ddouble,[int,string,double]);
share t as tglobal;
```

现在需要将myTable的改动同步到DolphinDB的tglobal中，
首先要连接到本地的DolphinDB服务器。
```sql
select dolInit('127.0.0.1',8848,'','');
```
接着用如下语句在MySQL中分别创建同步插入、删除、更新的触发器，
```sql
DELIMITER $
create trigger syncInsert after insert on myTable for each row
begin
	select dolInsert('tglobal','dint','dstr','ddouble',new.int,new.str,new.double) into @t1;
end $
create trigger syncDelete after delete on myTable for each row
begin
	select dolDelete('tglobal','dint',old.int) into @t2;
end $
create trigger syncUpdate after update on myTable for each row
begin
	select dolUpdate('tglobal',1,'dint',old.int,'dint','dstr','ddouble',new.int,new.str,new.double) into @t3;
end $
DELIMITER ;
```
之后表myTable中的增删改便会实时同步到DolphinDB的表tglobal中。

## 4. 其他

1. 预估同步性能在每秒数万行左右。
2. 目前仅支持同步MySQL的int，varchar，double三种数据类型到DolphinDB的内存表中。

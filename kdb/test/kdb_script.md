### kdb+启动

在kdb的启动文件中：
以用户名密码方式启动kdb+并指定监听端口为5000
./q -p 5000 -U ../passwordfiles/usrs 
> 注意-U一定是大写的！不然是local启动，client无法获取当前文件夹以上的文件

不指定用户名密码，仅仅指定监听端口为5000
./q -p 5000

### kdb+使用
https://code.kx.com/q/
https://www.zhihu.com/column/kdb2019
https://iowiki.com/kdbplus/kdbplus_index.html

### kdb+脚本
```q
/ 查看该表有哪些列
cols Daily

/ 强制类型转换
`long$8i

/ 以文件夹导入表，可以导入partition表，splayed表
\l /home/slshen/KDB/data/kdb_sample/
\l /home/slshen/KDB/data/kdb_sample/2022.06.17/Daily  
\l /home/slshen/KDB/data/kdb_sample/2022.06.17/Minute

/ 导入表并赋值
/注意不要在Daily后面加"/"，会使得api的读取发生错误
Daily:get`:/home/slshen/KDB/data/kdb_sample/2022.06.17/Daily

/ 赋值给sym表
sym:get`:/home/slshen/KDB/data/kdb_sample/sym

/ 将同名表存储在csv中
\t save `:/home/slshen/KDB/data/Minute.csv

/ 新建表
t:([] v1:10 20 30; v2:1.1 2.2 3.3)
t:([] v1:10 20; v2:1.1 2.2)
t:([] v1:10; v2:1.1)

/ 压缩存储表
(`:/home/slshen/KDB/data/Daily170206/;17;2;6) set Daily
/ 非压缩存储表
`:/home/slshen/KDB/data/splayed_Quotes/ set Quotes
/ 查看压缩相关参数
-21!`:/home/slshen/KDB/data//Daily170206/sym   

/ 制作测试用例
/ 压缩类型级别测试用例 ------------------------------------------------
/ 加载sym文件
sym:get`:/home/slshen/KDB/data/kdb_sample/sym
/ 加载客户数据
\l /home/slshen/KDB/data/kdb_sample/2022.06.17/Daily  
\l /home/slshen/KDB/data/kdb_sample/2022.06.17/Minute

`:/home/slshen/KDB/data/testDaily/Daily_plain/ set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170206/;17;2;6) set Daily
`:/home/slshen/KDB/data/testMinute/Minute_plain/ set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_170206/;17;2;6) set Minute

/ 不同的压缩块大小
(`:/home/slshen/KDB/data/testDaily/Daily_180206/;18;2;6) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_160206/;16;2;6) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_150206/;15;2;6) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_140206/;14;2;6) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_130206/;13;2;6) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_120206/;12;2;6) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_120205/;12;2;6) set Daily;
(`:/home/slshen/KDB/data/testDaily/Daily_120202/;12;2;6) set Daily;

(`:/home/slshen/KDB/data/testMinute/Minute_200206/;20;2;6) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_190206/;19;2;6) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_180206/;18;2;6) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_160206/;16;2;6) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_150206/;15;2;6) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_140206/;14;2;6) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_130206/;13;2;5) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_120206/;12;2;2) set Minute

/ 不同的压缩类型
(`:/home/slshen/KDB/data/testDaily/Daily_170000/;17;0;0) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170100/;17;1;0) set Daily
/ (`:/home/slshen/KDB/data/testDaily/Daily_170300/;17;3;0) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170406/;17;4;6) set Daily

(`:/home/slshen/KDB/data/testDaily/Daily_170200/;17;2;0) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170201/;17;2;1) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170202/;17;2;2) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170203/;17;2;3) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170204/;17;2;4) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170205/;17;2;5) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170207/;17;2;7) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170208/;17;2;8) set Daily
(`:/home/slshen/KDB/data/testDaily/Daily_170209/;17;2;9) set Daily

(`:/home/slshen/KDB/data/testMinute/Minute_170200/;17;2;0) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_170201/;17;2;1) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_170202/;17;2;2) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_170203/;17;2;3) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_170204/;17;2;4) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_170205/;17;2;5) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_170207/;17;2;7) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_170208/;17;2;8) set Minute
(`:/home/slshen/KDB/data/testMinute/Minute_170209/;17;2;9) set Minute


/ 数据类型测试用例 --------------------------------------------------
/ 新建不同类型的各列
col_bool:`boolean$(1;0;1;0b;0)
col_uuid:`guid$("G"$"61f35174-90bc-a48a-d88f-e15e4a377ec8";"G"$"61f35174-90bc-a48a-d88f-e15e4a377ec8";0Ng;"G"$"61f30174-90bc-a48a-d88f-e15e4a377ec8";"G"$"71f35174-90bc-a48a-d88f-e15e4a377ec8")
col_byte:`byte$(127;6;0;128;255)
col_short:`short$(34;-3;0;0Nh;100)
col_int:`int$(12;44535;0;0Ni;-234566)
col_long:`int$(23243;77347535;0;0Nj;-2346)
col_float:`real$(0;-232.0;345345;0.00023;0Ne)
col_double:`float$(0.0000000;0.00034350034;96778;0Nf;-1.2)
col_char:`char$("c";"0";"|";0Nc;" ")
col_str:`symbol$`ww`787`uih_`ww`
col_nanostamp:`timestamp$(1997.06.26D00:00:00.100000003;2008.01.01D00:00:00.100000003;2023.01.01D00:00:00.100000003;1970.01.01D00:00:00.100000003;0Np)
col_month:`month$(2000.01m;2000.01m;2000.12m;1900.01m;0Nm)
col_date:`date$(1997.06.26;2008.01.01;2023.01.01;1970.01.01;0Nd)
col_timestamp:`datetime$(1997.06.26T23:19:12.001;2008.01.01T23:19:12.001;2023.01.01T23:19:12.001;1970.01.01T23:19:12.001;0Nz)
col_nano:`timespan$(00:00:00.100000003;00:30:00.000300000;10:00:00.000000000;00:00:00.000000000;0Nn)
col_minute:`minute$(14:00;01:22;0Nu;13:01;03:41)
col_second:`second$(23:59:59;12:12:12;01:55:01;0Nv;00:00:00)
col_time:`time$(0Nt;00:01:01.978;23:19:12.001;12:00:01.030;00:00:00.000)

/ 建立表
t:([] bool:col_bool; uuid:col_uuid; byte:col_byte; short:col_short; int:col_int; long:col_long; float:col_float; double:col_double; char:col_char; str:col_str; nanostamp:col_nanostamp; month:col_month; date:col_date; timestamp:col_timestamp; nano:col_nano; minute:col_minute; second:col_second; time:col_time)

/ 把表中的字符枚举到sym中
.Q.en[`:/home/slshen/KDB/data/test/;t]
/ 把表中的字符进行枚举，同时存储表格到磁盘上
`:/home/slshen/KDB/data/test/types/ set .Q.en[`:/home/slshen/KDB/data/test/;t]
(`:/home/slshen/KDB/data/test/compressTypes/;17;2;6) set .Q.en[`:/home/slshen/KDB/data/test/;t]
```



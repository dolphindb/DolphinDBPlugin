#  Tests for odbc plugin

##  Testing environment setup
###  1. install database instances
[https://www.digitalocean.com/community/tutorials/how-to-install-mysql-on-ubuntu-16-04](https://www.digitalocean.com/community/tutorials/how-to-install-mysql-on-ubuntu-16-04)  
[https://www.digitalocean.com/community/tutorials/how-to-install-and-use-postgresql-on-ubuntu-16-04](https://www.digitalocean.com/community/tutorials/how-to-install-and-use-postgresql-on-ubuntu-16-04)  
[https://www.microsoft.com/en-us/sql-server/sql-server-editions-express](https://www.microsoft.com/en-us/sql-server/sql-server-editions-express)

###  2. importing testing database
* MySQL
```
git clone https://github.com/zxjcarrot/test_db
cd test_db
mysql -u root -p < employees_partitioned.sql
```
* Postgresql  
use [pgadmin](https://www.pgadmin.org/) to restore backup file ```postgres.tar```.  

* SQL Server  
use [SQL Server Management Studio](https://docs.microsoft.com/en-us/sql/ssms/download-sql-server-management-studio-ssms) to restore backup file ```mssql-employees.bak```
### 3. Install ODBC drivers
```
#install mysql odbc driver
apt-get install libmyodbc
#install postgresql odbc driver
apt-get install odbc-postgresql
```
For ```mssql``` odbc drivers, check [this](https://docs.microsoft.com/en-us/sql/connect/odbc/linux-mac/installing-the-microsoft-odbc-driver-for-sql-server) out.
> If you're having trouble installing ```libmyodbc``` on Ubuntu 16.04, try [this](https://www.datasunrise.com/blog/how-to-install-the-mysql-odbc-driver-on-ubuntu-16-04/).

### 4. Configure ODBC datasource   
Add follwing data source configurations to ```/etc/odbc.ini```.
```
[mysql-employees]
Driver={Your driver name}
SERVER=127.0.0.1
UID=root
PWD={Your Password}
PORT=3306
DATABASE=employees

[postgresql-employees]
Driver={Your driver name}
Servername=127.0.0.1
Port=5432
UserName=postgres
Password=postgres
Database=postgres

[mssql-employees]
Driver={Your driver name}
Server=192.168.1.35
Port=1433
Username=sa
Password=sa
Database=employees
```

Look up ```/etc/odbcinst.ini``` for driver names and replace values to match your own environment.

You can use follwing command to see if the setup is properly done.
```
# connect to mysql-employees data source using isql tool
isql mysql-employees
```

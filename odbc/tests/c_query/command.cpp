#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include "cvt.h"

using std::cout;
using std::cin;
using std::endl;
using std::string;

struct DataBinding {
   SQLSMALLINT TargetType;
   SQLPOINTER TargetValuePtr;
   SQLINTEGER BufferLength;
   SQLLEN StrLen_or_Ind;
};

void sqlerr(SQLSMALLINT HandleType, SQLHANDLE handle)
{
    SQLCHAR state[32];
    SQLINTEGER native;
    SQLCHAR message[255];
    SQLGetDiagRec(HandleType, handle, 1, state, &native, message, 255, NULL);
    printf("state = %s, message = %s\n", state, message);
}

int main()
{
    SQLHANDLE hEnv;
    SQLHANDLE hDbc;
    SQLHANDLE hStmt;
    int ret;

    //1．    分配ODBC环境句柄
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    if (!SQL_SUCCEEDED(ret))
    {
        printf("allocate env handle error\n");
        return -1;
    }

    //2．    设置环境
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(ret))
    {
        printf("set env error\n");
        sqlerr(SQL_HANDLE_ENV, hEnv);
        return -1;
    }

    //3．    分配ODBC连接句柄
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    if (!SQL_SUCCEEDED(ret))
    {
        printf("allocate connection handle error\n");
        sqlerr(SQL_HANDLE_ENV, hEnv);
        return -1;
    }
    bool flag = true;
    while(flag) {
        cout << "Your DSN str: \n  For example: DSN=mysql \n";
        string dsn;
        getline(cin, dsn);
        if(dsn == "exit" || dsn == "quit") {
            flag = false;
            break;
        }
        //4．    连接数据库
        ret = SQLDriverConnect(hDbc,
                NULL,
                (SQLCHAR*)(dsn.c_str()),
                SQL_NTS,
                NULL,
                0,
                NULL,
                SQL_DRIVER_COMPLETE);
        if (!SQL_SUCCEEDED(ret))
        {
            printf("connect error\n");
            sqlerr(SQL_HANDLE_DBC,hDbc);
            continue;
        }

        printf("connected\n");

        //5．    分配语句句柄
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        if (!SQL_SUCCEEDED(ret))
        {
            printf("allocate statement handle error\r\n");
            sqlerr(SQL_HANDLE_DBC,hDbc);
            return -1;
        }
        break;
    }
    flag = true;
    bool isUtf8 = false;
    while(flag) {
        cout << "utf8 or utf16, if utf8 type yes, if utf16 type no \n";
        string dsn;
        std::transform(dsn.begin(), dsn.end(), dsn.begin(), ::tolower);
        getline(cin, dsn);
        if(dsn == "yes" || dsn == "y" || dsn == "ye") {
            isUtf8 = true;
            break;
        } else if (dsn == "no" || dsn == "n") {
            isUtf8 = false;
            break;
        } 
    }

    //6．    执行指定的语句
    flag = true;
    while (flag) {
        cout << "Your SQL statement : \n";
        string stat;
        getline(cin, stat);
        if(stat == "exit" || stat == "quit") {
            break;
        }
        if(isUtf8) {
            cout << "utf8: "<< endl;
            ret = SQLExecDirect(hStmt,(SQLCHAR*)(stat.c_str()), SQL_NTS);
        } else {
            cout << "utf16: "<< endl;
            ret = SQLExecDirectW(hStmt,(SQLWCHAR*)(utf8_to_utf16(stat).c_str()), SQL_NTS);
        }

        if (!SQL_SUCCEEDED(ret))
        {
            printf("execute error\n");
            sqlerr(SQL_HANDLE_STMT, hStmt);
            continue;
        }
        //7．    取得结果集的列数
        SQLSMALLINT sNumResults;
        ret = SQLNumResultCols(hStmt, &sNumResults);
        if (!SQL_SUCCEEDED(ret))
        {
            printf("get result error\n");
            sqlerr(SQL_HANDLE_STMT, hStmt);
            continue;
        }

        //8．    取得结果集的行数
        SQLLEN cRowCount;
        ret = SQLRowCount(hStmt, &cRowCount);
        if (!SQL_SUCCEEDED(ret))
        {
            printf("allocate connection handle error\n");
            sqlerr(SQL_HANDLE_STMT, hStmt);
            continue;
        }
        //9.获取列名

        for(int i = 0; i < sNumResults; i++)
        {
            SQLCHAR colName[64];
            ret = SQLDescribeCol(hStmt, i + 1, colName, 64, NULL, NULL, NULL, NULL, NULL);
            if(!SQL_SUCCEEDED(ret))
            {
                printf("get column name error\n");
                sqlerr(SQL_HANDLE_STMT, hStmt);
                continue;
            }
            printf("%s\t", colName);
        }
        printf("\n");
        //10．    将缓冲区提交驱动程序进行绑定。
        struct DataBinding* columnData = (struct DataBinding*)malloc( sNumResults * sizeof(struct DataBinding) );
        int bufferSize = 1024;
        for (int i = 0 ; i < sNumResults ; i++ ) {
            columnData[i].TargetType = SQL_C_CHAR;
            columnData[i].BufferLength = (bufferSize+1);
            columnData[i].TargetValuePtr = malloc( sizeof(unsigned char)*columnData[i].BufferLength );
        }
        for (int i = 0 ; i < sNumResults ; i++ ) {
            ret = SQLBindCol(hStmt, (SQLUSMALLINT)i + 1, columnData[i].TargetType,
                columnData[i].TargetValuePtr, columnData[i].BufferLength, &(columnData[i].StrLen_or_Ind));
                if(!SQL_SUCCEEDED(ret))
                {
                    printf("bind col error\n");
                    sqlerr(SQL_HANDLE_STMT, hStmt);
                    continue;
                }
        }
        string printData;
        std::cout<<"print data? yes or no"<<std::endl;
        getline(cin, printData);
        bool printFlag;
        if(printData == "no" || printData == "n") {
            printFlag = false;
        } else {
            printFlag = true;  
        }
            //11．    调用SQLFetch提取数据，直至返回SQL_NO_DATA_FOUND。驱动程序将提取的数据写入上一步绑定的缓冲区中，供应用程序访问
        int rows = 1;
        for (ret = SQLFetch(hStmt); ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO; ret = SQLFetch(hStmt)) {
            if(printFlag) {
                for (int j = 0; j < sNumResults; j++) {
                    printf("%s\t", (char *)columnData[j].TargetValuePtr);
                }
                printf("\n");
            } else {
                if (rows % 10000 == 0) {
                    std::cout << "get " << rows << " row data" << std::endl;
                }
            }
            rows++;
        }
        printf( "\n" );
            //12．    处理完成后，释放所有资源

        for (int i = 0 ; i < sNumResults ; i++ ) {
            free(columnData[i].TargetValuePtr);
        }
        free(columnData);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    return 0;
}

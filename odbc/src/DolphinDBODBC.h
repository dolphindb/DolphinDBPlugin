/*
 * DolphinDBODBC.h
 *
 *  Created on: May 26, 2017
 *      Author: xinjing.zhou
 */

#ifndef DOLPHINDBODBC_H_
#define DOLPHINDBODBC_H_

#include "CoreConcept.h"
#include "Util.h"
#ifndef LINUX
#include <windows.h>
#endif
#include <sql.h>
#include <sqlext.h>
#include <cassert>
#include <climits>
#include <cstdio>
#include <iostream>
#include <locale>
#include <map>

#include <vector>
#include "cvt.h"
#include "nanodbc/nanodbc.h"

#include <fstream>
#include <iostream>

extern "C" ConstantSP odbcConnect(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP odbcClose(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP odbcQuery(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP odbcExecute(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP odbcAppend(Heap* heap, vector<ConstantSP>& args);

class OdbcConnection{
public:
    OdbcConnection(const nanodbc::connection& connection, const string& dataBaseType):connection_(connection),dataBaseType_(dataBaseType){}
    nanodbc::connection* getConnection(){return &connection_;};
    string getDataBaseType(){return dataBaseType_;}
private:
    nanodbc::connection connection_;
    string dataBaseType_;
};

#endif /* DOLPHINDBODBC_H_ */

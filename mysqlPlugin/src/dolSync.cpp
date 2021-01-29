#include <mysql/mysql.h>
#include <mysql/mysql_com.h>
#include "DolphinDB.h"
#include "Concurrent.h"
#include "Util.h"
#include <iostream>
#include <string>
#include <string.h>
#include <unordered_map>
#include <sstream>
using namespace dolphindb;
using namespace std;

extern "C"
{
    bool dolInit_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    long long dolInit(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);

    bool dolInsert_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    long long dolInsert(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);

    bool dolDelete_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    long long dolDelete(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);

    bool dolUpdate_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    long long dolUpdate(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
}

string doubleToString(const double value)
{
    ostringstream out;
    out.precision(16);
    out << value;
    return out.str();
}

DBConnection conn;

enum TriggerType
{
    invalidTrigger = -1,
    insertTrigger = 0,
    deleteTrigger = 1,
    updateTrigger = 2
};

struct SyncTask
{
    string tableName;
    TriggerType triType;
    vector<string> newName;
    vector<ConstantSP> newValue;
    string script;
    SyncTask(string name, TriggerType type) : tableName(name), triType(type) {}
};

typedef SmartPointer<SyncTask> SyncTaskSP;

class MysqlSyncTask : public Runnable
{
public:
    void run() override;
    void push(const SyncTaskSP &task) { queue_.push(task); }

private:
    SynchronizedQueue<SyncTaskSP> queue_;
};

SmartPointer<MysqlSyncTask> Synchronizer = new MysqlSyncTask();
ThreadSP mysqlSyncThread = new Thread(Synchronizer);

void MysqlSyncTask::run()
{
    while (true)
    {
        vector<SyncTaskSP> vec;
        auto cnt = queue_.size();
        if (cnt == 0)
        {
            SyncTaskSP p;
            if (!queue_.blockingPop(p, 1000))
                continue;
            vec.emplace_back(p);
        }
        else
            queue_.pop(vec, 1024);
        int vecSize = vec.size();
        if (vecSize <= 0)
            continue;
        unordered_map<string, pair<int, int>> map;
        int tableCnt = 0;
        for (int i = 0; i < vecSize; i++)
        {
            auto it = map.find(vec[i]->tableName);
            if (it == map.end())
            {
                map[vec[i]->tableName] = make_pair(tableCnt, -1);
                tableCnt++;
            }
            if (vec[i]->triType == insertTrigger)
                map[vec[i]->tableName].second = i;
        }
        vector<vector<VectorSP>> colVectors(tableCnt);
        for (int i = 0; i < vecSize; i++)
        {
            int tableId = map[vec[i]->tableName].first;
            if (vec[i]->triType == insertTrigger)
            {
                if (colVectors[tableId].empty())
                {
                    for (int j = 0; j < vec[i]->newValue.size(); j++)
                    {
                        colVectors[tableId].push_back(Util::createVector(vec[i]->newValue[j]->getType(), 0, 1e4));
                    }
                }
                for (int j = 0; j < vec[i]->newValue.size(); j++)
                {
                    colVectors[tableId][j]->append(vec[i]->newValue[j]);
                }
            }
            else
            {
                if (!colVectors[tableId].empty())
                {
                    vector<ConstantSP> newCol(colVectors[tableId].size());
                    for (int j = 0; j < colVectors[tableId].size(); j++)
                        newCol[j] = colVectors[tableId][j];
                    TableSP table = Util::createTable(vec[map[vec[i]->tableName].second]->newName, newCol);
                    vector<ConstantSP> tableArgs;
                    tableArgs.push_back(table);
                    string str = "tableInsert{" + vec[i]->tableName + "}";
                    conn.run(str, tableArgs);
                    colVectors[tableId].clear();
                }
                conn.run(vec[i]->script);
            }
        }
        for (auto m : map)
        {
            int i = m.second.first;
            if (!colVectors[i].empty())
            {
                vector<ConstantSP> newCol(colVectors[i].size());
                for (int j = 0; j < colVectors[i].size(); j++)
                    newCol[j] = colVectors[i][j];
                TableSP table = Util::createTable(vec[m.second.second]->newName, newCol);
                vector<ConstantSP> tableArgs;
                tableArgs.push_back(table);
                string str = "tableInsert{" + m.first + "}";
                conn.run(str, tableArgs);
            }
        }
    }
}

extern "C" bool dolInit_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{ //ip,port,userName,pwd
    if (args->arg_count != 4)
    {
        strcpy(message, "dolInit() requires four arguments");
        return 1;
    }
    if (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != INT_RESULT || args->arg_type[2] != STRING_RESULT || args->arg_type[3] != STRING_RESULT)
    {
        strcpy(message, "dolInit() arguments type error");
    }
    bool ret = conn.connect(string(args->args[0]), *((long long *)args->args[1]), string(args->args[2]), string(args->args[3]));
    if (!ret)
    {
        strcpy(message, "Failed to connect to the dolphindb server");
        return 1;
    }
    return 0;
}

extern "C" long long dolInit(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
    mysqlSyncThread->start();
    return 0;
}

extern "C" bool dolInsert_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    if (args->arg_count < 3)
    {
        strcpy(message, "dolInsert() requires at least three arguments");
        return 1;
    }
    if (args->arg_type[0] != STRING_RESULT)
    {
        strcpy(message, "table name must be string");
        return 1;
    }
    int size = ((args->arg_count) - 1) / 2;
    for (int i = 1; i < size + 1; i++)
    {
        if (args->arg_type[i] != STRING_RESULT)
        {
            strcpy(message, string("column name " + to_string(i) + " must be string").c_str());
            return 1;
        }
    }
    return 0;
}

extern "C" long long dolInsert(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{ //tableName,colName1...colNamen,new.colName1...new.colNamen
    int size = ((args->arg_count) - 1) / 2;
    SyncTaskSP insert = new SyncTask(string(args->args[0]), insertTrigger);
    insert->newName.reserve(size);
    insert->newValue.reserve(size);
    for (int i = 1; i <= size; i++)
    {
        insert->newName.push_back(string(args->args[i]));
        int valueIndex = i + size;
        switch (args->arg_type[valueIndex])
        {
        case INVALID_RESULT:
            return 1;
        case INT_RESULT:
            insert->newValue.push_back(Util::createInt(*((long long *)args->args[valueIndex])));
            break;
        case STRING_RESULT:
            insert->newValue.push_back(Util::createString(string(args->args[valueIndex])));
            break;
        case REAL_RESULT:
            insert->newValue.push_back(Util::createDouble(*((double *)args->args[valueIndex])));
            break;
        default:
            return 1;
        }
    }
    Synchronizer->push(insert);
    return 0;
}

bool dolDelete_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    if (args->arg_count < 3)
    {
        strcpy(message, "dolDelete() requires at least three arguments");
        return 1;
    }
    if (args->arg_type[0] != STRING_RESULT)
    {
        strcpy(message, "table name must be string");
        return 1;
    }
    int size = ((args->arg_count) - 1) / 2;
    for (int i = 1; i < size + 1; i++)
    {
        if (args->arg_type[i] != STRING_RESULT)
        {
            strcpy(message, string("column name " + to_string(i) + " must be string").c_str());
            return 1;
        }
    }
    return 0;
}
long long dolDelete(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
    int size = ((args->arg_count) - 1) / 2;
    SyncTaskSP task = new SyncTask(string(args->args[0]), deleteTrigger);
    task->script = "delete from " + string(args->args[0]) + " where ";
    for (int i = 1; i <= size; i++)
    {
        task->script += (string(args->args[i])) + "=";
        int valueIndex = i + size;
        switch (args->arg_type[valueIndex])
        {
        case INVALID_RESULT:
            return 1;
        case INT_RESULT:
            task->script += to_string(*((long long *)args->args[valueIndex]));
            break;
        case STRING_RESULT:
            task->script += "`" + string(args->args[valueIndex]);
            break;
        case REAL_RESULT:
            task->script += doubleToString(*((double *)args->args[valueIndex]));
            break;
        default:
            return 1;
        }
        task->script += " and ";
    }
    task->script.erase(task->script.end() - 5, task->script.end());
    Synchronizer->push(task);
    return 0;
}

bool dolUpdate_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{ //tableName,keyCnt,keyColName,keyColValue,newColName,newColValue
    if (args->arg_count < 5)
    {
        strcpy(message, "dolUpdate() requires at least five arguments");
        return 1;
    }
    if (args->arg_type[0] != STRING_RESULT)
    {
        strcpy(message, "table name must be string");
        return 1;
    }
    if (args->arg_type[1] != INT_RESULT)
    {
        strcpy(message, "number of primary keys must be integer");
        return 1;
    }
    return 0;
}
long long dolUpdate(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
    SyncTaskSP task = new SyncTask(string(args->args[0]), updateTrigger);
    int keyCnt = *((long long *)args->args[1]);
    int newColSize = (args->arg_count - 2 - keyCnt * 2) / 2;
    string script1;
    for (int i = 2; i < keyCnt + 2; i++)
    {
        script1 += (string(args->args[i])) + "=";
        int valueIndex = i + keyCnt;
        switch (args->arg_type[valueIndex])
        {
        case INVALID_RESULT:
            return 1;
        case INT_RESULT:
            script1 += to_string(*((long long *)args->args[valueIndex]));
            break;
        case STRING_RESULT:
            script1 += "`" + string(args->args[valueIndex]);
            break;
        case REAL_RESULT:
            script1 += doubleToString(*((double *)args->args[valueIndex]));
            break;
        default:
            return 1;
        }
        script1 += " and ";
    }
    script1.erase(script1.end() - 5, script1.end());
    vector<ConstantSP> vdebug;
    string script2 = " set ";
    for (int i = 2 + keyCnt * 2; i < 2 + keyCnt * 2 + newColSize; i++)
    {
        script2 += string(args->args[i]) + "=";
        int valueIndex = i + newColSize;
        switch (args->arg_type[valueIndex])
        {
        case INVALID_RESULT:
            return 1;
        case INT_RESULT:
            script2 += to_string(*((long long *)args->args[valueIndex]));
            break;
        case STRING_RESULT:
            script2 += "`" + string(args->args[valueIndex]);
            break;
        case REAL_RESULT:
            script2 += doubleToString(*((double *)args->args[valueIndex]));
            break;
        default:
            return 1;
        }
        script2 += ",";
    }
    script2.pop_back();
    task->script = "update " + string(args->args[0]) + script2 + " where " + script1;
    Synchronizer->push(task);
    return 0;
}
import pandas as pd
import os
import time
import pyarrow.feather as feather
from datetime import datetime

# 读取时间长度比较实验
class Timer:
    timers_start = dict()
    timers_end = dict()
    all_data = []

    @classmethod
    def reset(cls):
        Timer.timers_start = dict()
        Timer.timers_end = dict()
    
    @classmethod
    def printall(cls, path=None):
        dfstart = pd.DataFrame(Timer.timers_start)
        dfend = pd.DataFrame(Timer.timers_end)

        df = (dfend-dfstart)*1000
        if path is not None:
            dfa = df.T
            dfa.to_csv(path, sep=",")
        df = df.describe().T
        Timer.all_data.append(df)
        print(df)
        return df
        # print(Timer.timers)
    
    @classmethod
    def recordtime(cls, name, func, param):
        if name not in Timer.timers_start.keys():
            Timer.timers_start[name] = []
        if name not in Timer.timers_end.keys():
            Timer.timers_end[name] = []
        Timer.timers_start[name].append(time.time())

        func(param)

        
        Timer.timers_end[name].append(time.time())
        
    @classmethod
    def tocsv(cls, path):
        df = pd.concat(Timer.all_data, axis=0)
        df.to_csv(path, sep=",")

file_path = "/home/slshen/feather/data/"
file_name = "nostring_Minute.feather"

def testReadFeather(*arg):
    df = pd.read_feather(arg[0][0])
def testReadTable(*arg):
    df = feather.read_table(arg[0][0])

def testWriteFeather(*arg):
    feather.write_feather(arg[0][0], arg[0][1])

cus=feather.read_feather("/home/slshen/feather/data/cus.feather")
iter_times = 10
for i in range(iter_times):
    Timer.recordtime("10millonWrite", testWriteFeather, [cus, file_path + "tmp/cus.feather"])
    # Timer.recordtime("pa-trades_fread", testReadTable, [file_path + "Trades.lz4.feather"])
    # Timer.recordtime("pa-trades_small_fread", testReadTable, [file_path + "TradesSmall_lz4.feather"])
    # Timer.recordtime("pa-string_fread", testReadTable, [file_path + "df_string.feather"])

    # Timer.recordtime("trades_fread", testReadFeather, [file_path + "TradesSmall_lz4.feather"])
    # Timer.recordtime("trades_nostring_fread", testReadFeather, [file_path + "tmp/TradesSmall_zstd_no_string.feather"])
    # Timer.recordtime("df_bool_fread", testReadFeather, [file_path + "df_bool.feather"])
    # Timer.recordtime("df_int8_fread", testReadFeather, [file_path + "df_int8.feather"])
    # Timer.recordtime("df_uint8_fread", testReadFeather, [file_path + "df_uint8.feather"])
    # Timer.recordtime("df_int64_fread", testReadFeather, [file_path + "df_int64.feather"])
    # Timer.recordtime("df_uint64_fread", testReadFeather, [file_path + "df_uint64.feather"])
    # Timer.recordtime("df_float64_fread", testReadFeather, [file_path + "df_float64.feather"])
    # # Timer.recordtime("df_trades", testReadFeather, [file_path + "Trades.lz4.feather"])
    # Timer.recordtime("df-trades_small_fread", testReadFeather, [file_path + "TradesSmall_lz4.feather"])
    # Timer.recordtime("df_string_fread", testReadFeather, [file_path + "df_string.feather"])
    # Timer.recordtime("nostring_fread", testReadFeather, [file_path + "nostring_Minute.feather"])

Timer.printall()

def rw_feather(file_path, file_name):
    feather_file = file_path + file_name
    start = time.time()
    df = pd.read_feather(feather_file)
    end = time.time()
    c = end-start
    print("read: " + file_name + ": ", end='')
    print(c, end=' s\n')
    feather_file = file_path + "tmp" + file_name
    start = time.time()
    df.to_feather(feather_file)
    end = time.time()
    c = end-start
    print("write: " + file_name + ": ", end='')
    print(c, end=' s\n')

# """
# python读取csv文件时间长度对比
# """
# def read_csv(file_path, file_name):
#     csv_file = file_path+ file_name
#     start = time.time()
#     csv_df = pd.read_csv(csv_file)
#     end = time.time()
#     c = end-start
#     print("read: " + file_name + ": ", end='')
#     print(c, end=' s\n')
#     return csv_df

# def write_csv(df, file_path, file_name):
#     csv_file = file_path + file_name
#     start = time.time()
#     df.to_csv(csv_file)
#     end = time.time()
#     c = end-start
#     print("write: " + file_name + ": ", end='')
#     print(c, end=' s\n')

# """
# python读取feather文件时间长度对比
# """
# def read_feather(file_path, file_name):
#     feather_file = file_path + file_name
#     start = time.time()
#     df = pd.read_feather(feather_file)
#     end = time.time()
#     c = end-start
#     print("read: " + file_name + ": ", end='')
#     print(c, end=' s\n')
#     return df

# def write_feather(df, file_path, file_name):
#     feather_file = file_path + file_name
#     start = time.time()
#     df.to_feather(feather_file)
#     end = time.time()
#     c = end-start
#     print("write: " + file_name + ": ", end='')
#     print(c, end=' s\n')

# DIR = "/home/slshen/feather/data/"
# read_csv(DIR, "Trades_small.csv")
# read_csv('/home/slshen/KDB/data/tmp/', 'Minute.csv')
# t1 = read_feather(DIR, "Minute.feather")
# t2 = read_feather(DIR, "Trades_small.compressed.feather")
# t3 = read_feather(DIR, "nostring_Minute.feather")

# write_csv(t1, DIR, "tmp/Minute.csv")
# write_csv(t2, DIR, "tmp/Trades_small.csv")

# write_feather(t1, DIR, "tmp/Minute.feather")
# write_feather(t2, DIR, "tmp/Trades_small.feather")


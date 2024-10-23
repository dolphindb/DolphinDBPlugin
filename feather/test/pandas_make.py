import pandas as pd
import numpy as np
import pyarrow as pa
import pyarrow.feather as feather
from datetime import datetime
from datetime import date

np.random.seed = 2022
df_size = 10000000
file_path = "/home/slshen/feather/data/"

# 基础类型 ---------------------------------------
df_bool = pd.DataFrame({
    'a': np.ones(df_size, dtype=bool),
    })
df_bool.to_feather(file_path + "df_bool.feather")

df_int8 = pd.DataFrame({
    'a': np.ones(df_size, dtype=np.int8),
    })
df_int8.to_feather(file_path + "df_int8.feather")

df_int64 = pd.DataFrame({
    'a': np.arange(df_size),
    })
df_int64.to_feather(file_path + "df_int64.feather")

df_uint64 = pd.DataFrame({
    'a': np.ones(df_size, dtype=np.uint64),
    })
df_uint64.to_feather(file_path + "df_uint64.feather")

df_float64 = pd.DataFrame({
    'a': np.random.rand(df_size),
    })
df_float64.to_feather(file_path + "df_float64.feather")

df_float64x3 = pd.DataFrame({
    'a': np.random.rand(df_size),
    'b': np.random.rand(df_size),
    'c': np.random.rand(df_size)
    })
df_float64x3.to_feather(file_path + "df_float64x3.feather")

df_string = pd.DataFrame({
    'a':  np.array(['abbaaaabbabb' for _ in range(df_size)]),
    })
df_string.to_feather(file_path + "df_string.feather")


df_basic_type = pd.DataFrame({
    'double':   np.array([-35563.34523,-378785523,0,np.inf,np.nan]).astype(np.float64),
    'float':    np.array([909,0,-63.34523,10.45,np.nan]).astype(np.float32),
    'bool':     np.array([0,1,0,1,None]).astype(np.bool_),
    'int8':    np.array([11,23,0,100,np.nan]).astype(np.int8),

    'uint8':    pd.array([23,None,0,10,None], dtype=pd.UInt8Dtype()),
    'uint16':    pd.array([23,None,0,1040,None], dtype=pd.UInt16Dtype()),
    'uint32':    pd.array([3523,None,0,1040,None], dtype=pd.UInt32Dtype()),
    'uint64':    pd.array([355623,None,0,104570,None], dtype=pd.UInt64Dtype()),

    'int16':    np.array([-2,-23,0,100,np.nan]).astype(np.int16),
    'int32':    pd.array([-3556523,None,0,10450,None], dtype=pd.Int32Dtype()),
    # 'int64':    np.array([-3556334523,10,0,10450,np.int64()]).astype(np.int64),
    'int64':    pd.array([-3556334523,None,0,10450,None], dtype=pd.Int64Dtype())
    })

df_basic_type.to_feather(file_path + "df_basic_type.feather")

string1_type = pd.DataFrame({
    'string':   np.array(["dolphindb", "",'i',"65535", np.nan])
    })

string1_type.to_feather(file_path + "string1_type.feather")

# 时间和特殊类型 ---------------------------------------

#'string':   np.array(["dolphindb", "",np.nan,"65535"]),
# 'binary':   np.array(["dolphindb", "",np.nan,"65535"]).astype()

time_array = np.array(['2001-01-01T12:00', '2022-02-03T13:56:03.172', '2032-02-03T13:56:03.172',np.datetime64()], dtype='datetime64')

df_time_type = pd.DataFrame({
    'date32':   np.array(['2012-06-01', '2012-06-03', '1970-01-01', '2012-06-03', 'NaT'], dtype="datetime64"),
    'timestamp-ms':np.array([np.datetime64('2012-06-13T13:30:10.008'), np.datetime64('2012-06-13T13:30:10.010'), np.datetime64('2012-06-13T13:30:10.010'), np.datetime64('2012-06-13T13:30:10.010'), np.datetime64('NaT')]),
    'timestamp-ns':np.array([time_array[0], time_array[1], time_array[2],time_array[2], pd.NaT])
    # 'time32s':
    # 'time32ms':
    # 'time64ns':
    })

df_time_type.to_feather(file_path + "df_time_type.feather")


# 暂不支持类型 ---------------------------------------
#     'int8':
#     'uint16':
#     'uint32':
#     'uint64':
#     'half_float':
#     'half_float':
#     'fixed_size_binary':
#     'interval_months':
#     'interval_day_time':
#     'decimal128':
#     'decimal':
#     'decimal256':
#     'duration':
#     'large_string':
#     'large_binary':
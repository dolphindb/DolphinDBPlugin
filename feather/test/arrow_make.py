import pandas as pd
import numpy as np
import pyarrow as pa
import pyarrow.feather as feather
from datetime import datetime
from datetime import date
from datetime import time

np.random.seed = 2022
df_size = 1000000
file_path = "/home/slshen/feather/data/"

# 时间和特殊类型 ---------------------------------------

#'string':   np.array(["dolphindb", "",np.nan,"65535"]),
# 'binary':   np.array(["dolphindb", "",np.nan,"65535"]).astype()

time_array = np.array(['2001-01-01T12:00','2022-02-03T13:56:03.172','2022-02-03T13:56:03.172', '2032-02-03T13:56:03.172',np.datetime64('NaT')], dtype='datetime64')
pandas_time_array = pd.array([time_array[0], time_array[1], time_array[2], time_array[3], pd.NaT])

pa_time_base = pa.array([time_array[0], time_array[1], time_array[2], time_array[3], time_array[4]])


pa_time_time=pa.array([time(13,4,7), time(19,1,1), time(2,4,7), time(0,4,7), None])

pa_date32 = pa.compute.cast(pa_time_base, pa.date32())
pa_date64 = pa.compute.cast(pa_time_base, pa.date64())
pa_timestamp_ms = pa.compute.cast(pa_time_base, pa.timestamp('ms'))
# pa_timestamp_us = pa.compute.cast(pa_time_base, pa.timestamp('us'))
pa_timestamp_ns = pa.compute.cast(pa_time_base, pa.timestamp('ns'))

pa_time32ms = pa.compute.cast(pa_time_base, pa.time32('ms'))
pa_time32s = pa.compute.cast(pa_time_time, pa.time32('s'))
# pa_time64us = pa.compute.cast(pa_time_base, pa.time64('us'))
pa_time64ns = pa.compute.cast(pa_time_base, pa.time64('ns'))

name=['pa_date32','pa_date64','pa_timestamp_ms','pa_timestamp_ns','pa_time32s','pa_time32ms','pa_time64ns']
pa_time = pa.Table.from_arrays([pa_date32,pa_date64,pa_timestamp_ms,pa_timestamp_ns,pa_time32s,pa_time32ms,pa_time64ns], names=name)

feather.write_feather(pa_time, '/home/slshen/feather/data/pa_time_type.feather')

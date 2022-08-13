import pyarrow as pa
import numpy as np
import pandas as pd
import pyarrow.feather as feather

n=10

file_path = "/home/slshen/feather/data/"

cfloat_range = np.append(np.array([-1.25, 2.25, -3.45, 4.55, -5.78, np.NaN]), None)
# cfloat16 = pa.array(np.array(np.tile([1, 2, 3, 4, 5], int(n/5)), dtype="float16")[0:n], type=pa.float16())
cfloat32 = pa.array(np.array(np.tile(cfloat_range, int(np.ceil(n/len(cfloat_range)))), dtype="float32")[0:n], type=pa.float32())
cfloat64 = pa.array(np.array(np.tile(cfloat_range, int(np.ceil(n/len(cfloat_range)))), dtype="float64")[0:n], type=pa.float64())

data = [cfloat32, cfloat64]
colNames = ["cfloat32", "cfloat64"]
batch = pa.RecordBatch.from_arrays(data, colNames)
t = pa.Table.from_batches([batch])
feather.write_feather(t, file_path + "float_non.feather")

n=10000000
cint16_range = np.append(np.arange(-32768, 32768), None)
cint16 = pa.array(np.tile(cint16_range, int(np.ceil(n/len(cint16_range))))[0:n], type=pa.int16())
cuint16_range = np.append(np.arange(0, 65536), None)
cuint16 = pa.array(np.tile(cuint16_range, int(np.ceil(n/len(cuint16_range))))[0:n], type=pa.uint16())

data = [cint16, cuint16]
colNames = ["cint16", "cuint16"]
batch = pa.RecordBatch.from_arrays(data, colNames)
t = pa.Table.from_batches([batch])
feather.write_feather(t, file_path + "int16_non.feather")
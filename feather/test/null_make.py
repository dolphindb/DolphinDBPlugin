import pandas as pd
import numpy as np
import pyarrow as pa
import pyarrow.feather as feather
from datetime import datetime
from datetime import date

file_path = "/home/slshen/feather/data/"

df = pd.DataFrame({
    # "col1":np.arange(-100, 100, dtype="float32"),
    "col2":np.array(np.repeat([2.15, np.nan, 3.5, np.nan], 200/4), dtype="float32"),
    # "col3":np.array(np.repeat(np.nan, 200), dtype="float32")
})
feather.write_feather(df, file_path + "float_default.feather")

n = 2000
df = pd.DataFrame({
    "col1":np.repeat(True, n),
     "col2":np.repeat(False, n), 
     "col3":np.repeat([True, False], n/2), 
     "col4":np.repeat([True, None, False, None], n/4)})
feather.write_feather(df, file_path + "bool_default.feather")
#ifndef CONVERTER_TYPECONVERTER_H_
#define CONVERTER_TYPECONVERTER_H_

#include "Helper.h"
#include "TypeDefine.h"
#include "TypeException.h"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

namespace converter {


class PyCache {
public:
    // Python Environment Distinction
    bool np_above_1_20_{true};
    bool pd_above_2_0_{true};
    bool pd_above_1_2_{true};
    bool pyarrow_import_{false};
    bool has_arrow_{false};

    // modules
    py::module_ numpy_;                     // module   numpy
    py::module_ pandas_;                    // module   pandas
    py::module_ pyarrow_;                   // module   pyarrow
    py::module_ socket_;                    // module   socket
    py::module_ decimal_;                   // module   decimal
    py::module_ datetime_;                  // module   datetime

    // python types
    py::object py_none_;                    // None
    py::object py_bool_;                    // bool
    py::object py_int_;                     // int
    py::object py_float_;                   // float
    py::object py_str_;                     // str
    py::object py_bytes_;                   // bytes
    py::object py_set_;                     // set
    py::object py_tuple_;                   // tuple
    py::object py_list_;                    // list
    py::object py_dict_;                    // dict
    py::object py_slice_;                   // slice
    py::object py_decimal_;                 // decimal.Decimal
    py::object py_datetime_;                // datetime.datetime
    py::object py_date_;                    // datetime.date
    py::object py_time_;                    // datetime.time
    py::object py_timedelta_;               // datetime.timedelta

    // numpy types
    py::object np_array_;                   // np.ndarray
    py::object np_matrix_;                  // np.matrix

    // numpy dtypes
    py::object np_object_;                  // np.dtype("object")
    py::object np_bool_;                    // np.dtype("bool")
    py::object np_int8_;                    // np.dtype("int8")
    py::object np_int16_;                   // np.dtype("int16")
    py::object np_int32_;                   // np.dtype("int32")
    py::object np_int64_;                   // np.dtype("int64")
    py::object np_float32_;                 // np.dtype("float32")
    py::object np_float64_;                 // np.dtype("float64")
    py::object np_str_type_;                // type(np.dtype("str"))
    py::object np_bytes_type_;              // type(np.dtype("bytes"))
    py::object np_datetime64_type_;         // np.datetime64
    py::object np_float32_type_;            // np.float32

    py::object np_datetime64_;              // np.dtype("datetime64")
    py::object np_datetime64M_;             // np.dtype("datetime64[M]")
    py::object np_datetime64D_;             // np.dtype("datetime64[D]")
    py::object np_datetime64m_;             // np.dtype("datetime64[m]")
    py::object np_datetime64s_;             // np.dtype("datetime64[s]")
    py::object np_datetime64h_;             // np.dtype("datetime64[h]")
    py::object np_datetime64ms_;            // np.dtype("datetime64[ms]")
    py::object np_datetime64us_;            // np.dtype("datetime64[us]")
    py::object np_datetime64ns_;            // np.dtype("datetime64[ns]")

    // pandas types
    py::object pd_NaT_;                     // pd.NaT
    py::object pd_NA_;                      // pd.NA
    py::object pd_dataframe_;               // pd.DataFrame
    py::object pd_series_;                  // pd.Series
    py::object pd_index_;                   // pd.Index
    py::object pd_timestamp_;               // pd.Timestamp
    py::object pd_extension_dtype_;         // pd.core.dtypes.dtypes.ExtensionDtype

    // pandas extension dtypes
    py::object pd_BooleanDtype_;
    py::object pd_Float32Dtype_;
    py::object pd_Float64Dtype_;
    py::object pd_Int8Dtype_;
    py::object pd_Int16Dtype_;
    py::object pd_Int32Dtype_;
    py::object pd_Int64Dtype_;
    py::object pd_StringDtype_;

    // pyarrow types
    py::object pa_boolean_;                 // pa.bool_()
    py::object pa_int8_;                    // pa.int8()
    py::object pa_int16_;                   // pa.int16()
    py::object pa_int32_;                   // pa.int32()
    py::object pa_int64_;                   // pa.int64()
    py::object pa_float32_;                 // pa.float32()
    py::object pa_float64_;                 // pa.float64()
    py::object pa_date32_;                  // pa.date32()
    py::object pa_time32_s_;                // pa.time32("s")
    py::object pa_time32_ms_;               // pa.time32("ms")
    py::object pa_time64_ns_;               // pa.time64("ns")
    py::object pa_timestamp_s_;             // pa.timestamp("s")
    py::object pa_timestamp_ms_;            // pa.timestamp("ms")
    py::object pa_timestamp_ns_;            // pa.timestamp("ns")
    py::object pa_utf8_;                    // pa.utf8()
    py::object pa_large_utf8_;              // pa.large_utf8()
    py::object pa_dictionary_type_;         // pa.DictionaryType
    py::object pa_fixed_size_binary_16_;    // pa.binary(16)
    py::object pa_large_binary_;            // pa.large_binary()
    py::object pa_decimal128_;              // pa.Decimal128Type
    py::object pa_list_;                    // pa.list_(...)
    py::object pa_chunked_array_;           // pa.ChunkedArray
    py::object pa_table_;                   // pa.Table
    py::object pd_arrow_dtype_;             // pd.ArrowDtype

    py::object name2dtype(const char *pname){
        py::object type = py::reinterpret_borrow<py::object>(numpy_.attr("dtype")(pname));
        return type;
    }
    
public:
    PyCache();
};


bool checkInnerType(const py::handle &data, ConstantSP &constantsp);


class PyObjs {
public:
    static PyCache* cache_;
    static void Initialize() { if (cache_ == nullptr) cache_ = new PyCache(); }
};


enum CHILD_VECTOR_OPTION {
    NORMAL_VECTOR,
    ANY_VECTOR,
    ARRAY_VECTOR,
    SET_VECTOR,
    KEY_VECTOR,
};


enum ARRAY_OPTION {
    AO_UNSPEC,
    AO_VECTOR,
    AO_MATRIX,
};


struct VectorInfo {
public:
    enum OPTION {EMPTY, COLNAME, COLID};
public:
    OPTION      option;
    std::string colName;        // use for Table column
    size_t      colId;
    bool        isCol;          // use for matrix
public:
    VectorInfo(): option(VectorInfo::EMPTY), colName(""), colId(0), isCol(false) {}
    VectorInfo(const std::string &col): option(VectorInfo::COLNAME), colName(col), colId(0), isCol(false) {}
    VectorInfo(size_t id, bool is_col=true): option(VectorInfo::COLID), colName(""), colId(id), isCol(is_col) {}
};


struct ToPythonOption {
public:
    bool        table2List_;    // if object is table, false: convert to pandas, true: convert to list[numpy]
public:
    ToPythonOption(): table2List_(false) {}
    ToPythonOption(bool table2List): table2List_(table2List) {}
};


class TableChecker : public helper::InsertionOrderedDict<std::string, Type> {
public:
    TableChecker() {}
    TableChecker(const py::dict &pydict);
};


class Converter {
public:
    static ConstantSP toDolphinDB(const py::handle &data, const Type &type = {HT_UNK, EXPARAM_DEFAULT});
    static ConstantSP toDolphinDB_Scalar(const py::handle &data, Type type, bool checkInner=true);
    static ConstantSP toDolphinDB_Vector(const py::handle &data, Type type, const CHILD_VECTOR_OPTION &option, const VectorInfo &info = VectorInfo(), bool checkInner = true);
    static ConstantSP toDolphinDB_Matrix(const py::handle &data, Type type, bool checkInner = true);
    static ConstantSP toDolphinDB_Dictionary(const py::handle &data, Type keyType, Type valType, bool isOrdered = true, bool checkInner = true);
    static ConstantSP toDolphinDB_Set(const py::handle &data, Type type = {HT_UNK, EXPARAM_DEFAULT}, bool checkInner = true);
    static ConstantSP toDolphinDB_Table(const py::handle &data, const TableChecker &checker = TableChecker(), bool checkInner = true);
    template <typename T>
    static ConstantSP toDolphinDB_Vector_fromTupleorListorSet(
        const T &data, Type type, const CHILD_VECTOR_OPTION &option, const VectorInfo &info = VectorInfo(),
        typename std::enable_if<std::is_same<T, py::list>::value || std::is_same<T, py::tuple>::value || std::is_same<T, py::set>::value, T>::type* = nullptr
    );
    static ConstantSP toDolphinDB_VectorOrMatrix_fromNDArray(
        py::array data, Type type, const CHILD_VECTOR_OPTION &option,
        const ARRAY_OPTION &arrOption, const VectorInfo &info = VectorInfo()
    );
    static ConstantSP toDolphinDB_Vector_fromSeriesOrIndex(py::object data, Type type, const CHILD_VECTOR_OPTION &option, const VectorInfo &info);
    template <typename T>
    static ConstantSP toDolphinDB_Matrix_fromTupleOrListOrNDArray(
        const T &data, Type type,
        typename std::enable_if<std::is_same<T, py::list>::value || std::is_same<T, py::tuple>::value || std::is_same<T, py::array>::value, T>::type* = nullptr
    );
    static ConstantSP toDolphinDB_Table_fromDataFrame(const py::object &data, const TableChecker &checker);
    static ConstantSP toDolphinDB_Table_fromDict(const py::dict &data, const TableChecker &checker);
    static ConstantSP toDolphinDB_Dictionary_fromDict(const py::dict &data, Type keyType = {HT_UNK, EXPARAM_DEFAULT}, Type valType = {HT_UNK, EXPARAM_DEFAULT}, bool isOrdered = true);
    static ConstantSP toDolphinDB_Set_fromSet(const py::set &data, Type type = {HT_UNK, EXPARAM_DEFAULT});

    static py::object toPython(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());

    static py::object toPython_Scalar(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());

    static py::list toPyList_Vector(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());
    static py::list toPyList_Matrix(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());
    static py::list toPyList_Table(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());

    static py::dict toPyDict_Table(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());
    static py::dict toPyDict_Dictionary(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());

    static py::set toPySet_Set(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());

    static py::object toNumpy_Vector(const ConstantSP &data, const ToPythonOption &option = ToPythonOption(), bool hasNull = false);
    static py::object toNumpy_Matrix(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());

    static py::object toPandas_Vector(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());
    static py::object toPandas_Table(const ConstantSP &data, const ToPythonOption &option = ToPythonOption());

private:
    std::unordered_map<std::string, HELPER_TYPE> typeMap_;
};


#define CHECK_INS(data, type)       py::isinstance(data, converter::PyObjs::cache_->type)
#define CHECK_EQUAL(data, type)     (data.equal(converter::PyObjs::cache_->type))



int getPyDecimalScale(const py::handle &obj);

int getPyDecimalScale(PyObject* obj);


template <typename T>
T getPyDecimalData(const py::handle &data, bool &hasNull, int scale) {
    throw pybind_dolphindb::ConversionException("Only c++ int, long, or int128 types can be converted to python Decimal.");
}

template <>
int getPyDecimalData<int>(const py::handle &data, bool &hasNull, int scale);

template <>
long long getPyDecimalData<long long>(const py::handle &data, bool &hasNull, int scale);

template <>
int128 getPyDecimalData<int128>(const py::handle &data, bool &hasNull, int scale);

template <typename T>
void getDecimalDigits(T raw_data, std::vector<int> &digits);





}




#endif

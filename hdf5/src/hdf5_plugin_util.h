#ifndef HDF5_PLUGIN_UTIL_H
#define HDF5_PLUGIN_UTIL_H

#include <CoreConcept.h>
#include <Logger.h>
#include <H5Cpp.h>
#include <hdf5_hl.h>
#include <blosc_filter.h>
/* HELPERS */

const string HDF5_LOG_PREFIX = "[PLUGIN::HDF5] ";

#define HDF5_SAFE_EXECUTE(line)       \
    try {                             \
        line;                         \
    } catch (std::exception& ex) {    \
        throw RuntimeException(HDF5_LOG_PREFIX + __FUNCTION__ + ": hdf5 execute \"" #line "\" failed due to " + ex.what()); \
    } catch (...) {                                                                                                         \
        throw RuntimeException(HDF5_LOG_PREFIX + __FUNCTION__ + ": hdf5 execute \"" #line "\" failed.");                    \
    }

bool colsNumEqual(const TableSP& t, int nCols);
void generateIncrementedColsName(std::vector<string> &cols, int size);
std::string typeIncompatibleErrorMsg(int idx, DATA_TYPE src, const VectorSP& destVec);
void registerUnixTimeConvert();
void checkHDF5Parameter(Heap* heap, vector<ConstantSP> &arguments, ConstantSP &filename, ConstantSP& destOrGroupName,
                        ConstantSP &schema, size_t& startRow, size_t& rowNum, const string &syntax);
void checkFailAndThrowRuntimeException(bool val, const string &errMsg);
void checkFailAndThrowRuntimeException(bool left, bool right, const string &errMsg);
void checkFailAndThrowIOException(bool val, const string &errMsg, IO_ERR errCode=OTHERERR);

/* AUX */
typedef struct GroupInfoTag
{
    string dataType;
    vector<string> *colsName{};
    vector<string> *kindColsName{};

    GroupInfoTag(string &type, vector<string> *name, size_t indexCount) : dataType(type), colsName(name) {}

    GroupInfoTag() = default;
} GroupInfo;

class InitHdf5Filter{
public:
    InitHdf5Filter(){
        try{
            int r = register_blosc(&version_, &date_);
            if(r < 0)
                LOG_ERR(HDF5_LOG_PREFIX + "Failed to register blosc filter. ");
            else
                LOG_INFO(HDF5_LOG_PREFIX + "Success to register blosc filter. Blosc version info : " + string(version_) + " " + string(date_));
        }catch(exception &e){
            LOG_ERR(e.what());
        }
    }
    ~InitHdf5Filter() {
        // TODO unregister blosc
        free(version_);
        free(date_);
    }
private:
    char* version_;
    char* date_;
};

#endif
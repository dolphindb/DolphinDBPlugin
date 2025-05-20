
#ifndef HDF5_PLUGIN_PANDAS_H
#define HDF5_PLUGIN_PANDAS_H
#include <CoreConcept.h>
#include <Util.h>
#include <ScalarImp.h>
#include <Logger.h>
#include <hdf5_plugin_util.h>
#include <hdf5_plugin_obj.h>

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;
using std::string;

namespace H5PluginImp
{
ConstantSP loadPandasHDF5(const string &fileName, const string &groupName, const ConstantSP &schema,const size_t startRow, const size_t rowNum);
ConstantSP loadPandasHDF5(const hid_t set, const ConstantSP &schema, const size_t startRow, const size_t rowNum, GroupInfo &info);
TableSP loadFrameTypeHDF5(const H5::Group& group, const ConstantSP& schema, size_t startRow, size_t readRowNum, const string& groupName);
TableSP loadSeriesTypeHDF5(const H5::Group& group, const ConstantSP &schema, size_t startRow, size_t readRowNum, const string& groupName);
}
#endif
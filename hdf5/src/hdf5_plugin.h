#ifndef HDF5_PLUGIN_H
#define HDF5_PLUGIN_H

#include <CoreConcept.h>

extern "C" ConstantSP h5ls(const ConstantSP &h5_path);
extern "C" ConstantSP h5lsTable(const ConstantSP &filename);
extern "C" ConstantSP extractHDF5Schema(const ConstantSP &filename, const ConstantSP &datasetName);
extern "C" ConstantSP loadHDF5(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadPandasHDF5(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadHDF5Ex(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP HDF5DS(Heap *heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP saveHDF5(Heap *heap, vector<ConstantSP> &arguments);


#endif /* HDF5_PLUGIN_H */

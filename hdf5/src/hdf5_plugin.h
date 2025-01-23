#ifndef HDF5_PLUGIN_H
#define HDF5_PLUGIN_H

#include <CoreConcept.h>

extern "C" ConstantSP h5ls(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP h5lsTable(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP extractHDF5Schema(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadHDF5(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadPandasHDF5(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadHDF5Ex(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP HDF5DS(Heap *heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP saveHDF5(Heap *heap, vector<ConstantSP> &arguments);


#endif /* HDF5_PLUGIN_H */

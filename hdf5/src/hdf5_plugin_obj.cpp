#include "hdf5_plugin_obj.h"

#include <hdf5_plugin_type.h>

namespace H5PluginImp {

// char *H5Object::getName(hid_t id)
// {
//     static char n[30];
//     int ret = H5Iget_name(id, n, 29);
//     return ret > 0 ? n : nullptr;
// }

hid_t H5ReadOnlyFile::open(const std::string &filename) {
    close();

    htri_t r = H5Fis_hdf5(filename.c_str());
    checkFailAndThrowIOException(r == 0, filename + " is not an HDF5 file", INVALIDDATA);
    checkFailAndThrowIOException(r < 0, "check " + filename + " failed,it may not exist");
    id_ = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    checkFailAndThrowIOException(id_ < 0, "can't open file" + filename);

    return id_;
}

void H5ReadOnlyFile::close() {
    if (valid()) H5Fclose(id_);

    id_ = H5I_INVALID_HID;
}

// H5DataSet imp

hid_t H5DataSet::open(const std::string &dset_name, hid_t loc_id) {
    close();
    id_ = H5Dopen(loc_id, dset_name.c_str(), H5P_DEFAULT);
    checkFailAndThrowIOException(id_ <= 0, "can't open dataset " + dset_name, INVALIDDATA);

    return id_;
}

void H5DataSet::close() {
    if (valid()) H5Dclose(id_);

    id_ = H5I_INVALID_HID;
}

// H5DatSpace imp

hid_t H5DataSpace::openFromDataset(hid_t dset_id) {
    id_ = H5Dget_space(dset_id);
    checkFailAndThrowIOException(id_<=0, "can't open dataspace", INVALIDDATA);

    return id_;
}

hid_t H5DataSpace::create(int rank, const hsize_t *current_dims, const hsize_t *maximum_dims) {
    id_ = H5Screate_simple(rank, current_dims, maximum_dims);
    checkFailAndThrowRuntimeException(id_<0, "create hdf5 dataspace failed");

    return id_;
}

void H5DataSpace::close() {
    if (valid()) H5Sclose(id_);

    id_ = H5I_INVALID_HID;
}

int H5DataSpace::rank() const { return H5Sget_simple_extent_ndims(id_); }

int H5DataSpace::currentDims(std::vector<hsize_t> &dims) const {
    int rank = this->rank();
    checkFailAndThrowRuntimeException(rank < 0, "Failed to get the rank in the dataspace.");

    dims.resize(rank);
    int ret = H5Sget_simple_extent_dims(id_, dims.data(), nullptr);
    checkFailAndThrowRuntimeException(ret < 0, "Failed to get the dimensions in the dataspace.");
    return ret;
}

// H5DataType imp

size_t H5DataType::size() const { return H5Tget_size(id_); }

hid_t H5DataType::openFromDataset(hid_t dset_id) {
    id_ = H5Dget_type(dset_id);
    checkFailAndThrowIOException(id_ <= 0, "can't open dataspace", INVALIDDATA);
    return id_;
}

void H5DataType::close() {
    if (valid()) H5Tclose(id_);
    id_ = H5I_INVALID_HID;
}

void H5Property::close() {
    if (valid()) H5Pclose(id_);

    id_ = H5I_INVALID_HID;
}

bool acceptNativeType(H5DataType &type, hid_t src_type) {
    return type.accept(H5Tget_native_type(src_type, H5T_DIR_ASCEND)) > 0;
}

}  // namespace H5PluginImp
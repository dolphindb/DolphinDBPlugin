#ifndef HDF5_PLUGIN_OBJ_H
#define HDF5_PLUGIN_OBJ_H

#include <CoreConcept.h>
#include <Logger.h>
#include <ScalarImp.h>
#include <Util.h>
#include <hdf5_plugin_util.h>

namespace H5PluginImp {
class H5Object {
  public:
    inline hid_t id() const { return id_; }
    // static char *getName(hid_t id);
    virtual void close(){};
    virtual ~H5Object() { close(); };

    hid_t accept(hid_t new_id) {
        close();
        return (id_ = new_id);
    }

    hid_t release() {
        hid_t old_id = id_;
        id_ = H5I_INVALID_HID;
        return old_id;
    }

    hid_t valid() { return H5Iis_valid(id_) > 0; }

  protected:
    hid_t id_ = H5I_INVALID_HID;
};

class H5ReadOnlyFile : public H5Object {
  public:
    H5ReadOnlyFile() = default;
    ;
    explicit H5ReadOnlyFile(const std::string &filename) { open(filename); }
    H5ReadOnlyFile(const H5ReadOnlyFile &rhs) = delete;
    H5ReadOnlyFile &operator=(const H5ReadOnlyFile &rhs) = delete;
    hid_t open(const std::string &filename);
    void close() override;
    ~H5ReadOnlyFile() { close(); }
};

class H5DataSet : public H5Object {
  public:
    H5DataSet() = default;
    H5DataSet(const std::string &dataSetName, hid_t loc_id) { open(dataSetName, loc_id); }
    H5DataSet(const H5DataSet &rhs) = delete;
    H5DataSet &operator=(const H5DataSet &rhs) = delete;
    hid_t open(const std::string &dataSetName, hid_t loc_id);
    void close() override;
    ~H5DataSet() { close(); }
};

class H5DataSpace : public H5Object {
  public:
    H5DataSpace() = default;
    explicit H5DataSpace(hid_t dataSetID) { openFromDataset(dataSetID); }
    H5DataSpace(int rank, const hsize_t *current_dims, const hsize_t *maximum_dims) {
        create(rank, current_dims, maximum_dims);
    }

    H5DataSpace(const H5DataSpace &rhs) = delete;
    H5DataSpace &operator=(const H5DataSpace &rhs) = delete;
    hid_t openFromDataset(hid_t dataSetID);
    hid_t create(int rank, const hsize_t *current_dims, const hsize_t *maximum_dims);
    int rank() const;
    int currentDims(std::vector<hsize_t> &dims) const;
    void close() override;
    ~H5DataSpace() { close(); }
};

class H5DataType : public H5Object {
  public:
    H5DataType() = default;
    H5DataType(hid_t id) { id_ = id; }  // memo: used in implicit conversion
    H5DataType(const H5DataType &rhs) = delete;
    H5DataType(H5DataType &&rhs) {
        this->accept(rhs.id());
        rhs.release();
    }

    H5DataType &operator=(const H5DataType &rhs) = delete;
    hid_t openFromDataset(hid_t dataSetID);
    size_t size() const;
    void close() override;
    ~H5DataType() { close(); }
};

class H5Property : public H5Object {
  public:
    H5Property() = default;
    explicit H5Property(hid_t id) { id_ = id; }
    H5Property(const H5Property &rhs) = delete;
    H5Property &operator=(const H5Property &rhs) = delete;
    void close() override;
    ~H5Property() { close(); }
};
enum h5_type_flag {
    IS_FIXED_STR = 0,
    IS_VARIABLE_STR,

    IS_ENUM,
    IS_COMPOUND,
    IS_ARRAY,

    IS_S_CHAR_INTEGER,
    IS_S_SHORT_INTEGER,
    IS_S_INT_INTEGER,
    IS_S_LLONG_INTEGER,

    IS_U_CHAR_INTEGER,
    IS_U_SHORT_INTEGER,
    IS_U_INT_INTEGER,
    IS_U_LLONG_INTEGER,

    IS_FLOAT_FLOAT,
    IS_DOUBLE_FLOAT,

    IS_UNIX_TIME,

    IS_BIT
};

struct hdf5_type_layout {
    std::string name;
    int offset;
    int belong_to;
    int flag;
    int size;
};

bool acceptNativeType(H5DataType &type, hid_t src_type);

inline bool isClassEqual(hid_t type, H5T_class_t target_class) { return H5Tget_class(type) == target_class; }

}  // namespace H5PluginImp
#endif
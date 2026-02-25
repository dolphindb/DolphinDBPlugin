#pragma once

#include <sys/stat.h>

#include <cstdio>
#include <fstream>

#include "DolphinDBEverything.h"

namespace ddb {

inline string getSessionUser(Heap *heap, const string &prefix = "") {
    FunctionDefSP func = Util::getFuncDefFromHeap(heap, "getCurrentSessionAndUser");
    if (func.isNull()) {
        throw RuntimeException(prefix + "Fail to get getCurrentSessionAndUser function.");
    }

    vector<ConstantSP> args{};
    ConstantSP info = func->call(heap, args);
    if (!info->isArray() || info->size() < 2 || info->get(1)->getType() != DT_STRING) {
        throw RuntimeException(prefix + "Fail to get current session user.");
    }
    return info->getString(1);
}

inline bool isAdminUser(Heap *heap, const string &userName, const string &prefix = "") {
    FunctionDefSP func = Util::getFuncDefFromHeap(heap, "getUserAccess");
    if (func.isNull()) {
        throw RuntimeException(prefix + "Fail to get getUserAccess function.");
    }

    vector<ConstantSP> args{new String(userName)};
    ConstantSP info = func->call(heap, args);
    if (!info->isTable() || info->columns() < 3 || info->size() != 1 ||
        reinterpret_cast<Table *>(info.get())->getColumnName(2) != "isAdmin" ||
        reinterpret_cast<Table *>(info.get())->getColumnType(2) != DT_BOOL) {
        throw RuntimeException(prefix + "Fail to get access info for user: " + userName + ".");
    }
    return info->getColumn(2)->getBool(0);
}

inline bool checkSessionUser(Heap *heap, const string &expectUser, const string &prefix = "", bool allowAdmin = true) {
    string currentUser = getSessionUser(heap, prefix);
    return (expectUser.empty() || currentUser == expectUser || (allowAdmin && isAdminUser(heap, currentUser, prefix)));
}

class Serializer {
  public:
    Serializer() = default;
    Serializer(DataOutputStreamSP out, const string &pluginName) : out_(out), pluginName_(pluginName) {}
    Serializer(DataInputStreamSP in, const string &pluginName) : in_(in), pluginName_(pluginName) {}
    Serializer(const string &filePath, bool forWriting, bool append = false, const string &pluginName = "")
        : pluginName_(pluginName) {
        if (forWriting) {
            FILE *file = (append) ? fopen(filePath.c_str(), "ab") : fopen(filePath.c_str(), "wb");
            if (!file)
                throw RuntimeException(pluginName_ + "Serialize failed: failed to open file for writing: " + filePath);
            out_ = new DataOutputStream(file, true);
        } else {
            readFromFile(filePath);
        }
    }
    DataOutputStreamSP getOutputStream() { return out_; }
    DataInputStreamSP getInputStream() { return in_; }
    void flush() {
        if (!out_.isNull()) out_->flush(true);
    }
    void close() {
        if (!out_.isNull()) {
            out_->flush(true);
            out_->close();
        }
    }

    // Write primitive types
    Serializer &operator<<(bool value) {
        IO_ERR ret = out_->write(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write bool failed");
        return *this;
    }
    Serializer &operator<<(char value) {
        IO_ERR ret = out_->write(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write char failed");
        return *this;
    }
    Serializer &operator<<(short value) {
        IO_ERR ret = out_->write(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write short failed");
        return *this;
    }
    Serializer &operator<<(int value) {
        IO_ERR ret = out_->write(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write int failed");
        return *this;
    }
    Serializer &operator<<(long long value) {
        IO_ERR ret = out_->write(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write long long failed");
        return *this;
    }
    Serializer &operator<<(uint32_t value) {
        IO_ERR ret = out_->write((long long)value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write uint32_t failed");
        return *this;
    }
#ifdef __linux__    
    Serializer &operator<<(int64_t value) {
        IO_ERR ret = out_->write((long long)value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write int64_t failed");
        return *this;
    }
#endif
    Serializer &operator<<(size_t value) {
        IO_ERR ret = out_->write((long long)value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write size_t failed");
        return *this;
    }
    Serializer &operator<<(float value) {
        IO_ERR ret = out_->write(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write float failed");
        return *this;
    }
    Serializer &operator<<(double value) {
        IO_ERR ret = out_->write(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write double failed");
        return *this;
    }

    // Write string with length prefix
    Serializer &operator<<(const string &value) {
        int len = static_cast<int>(value.length());
        IO_ERR ret = out_->write(len);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write string length failed");
        if (len > 0) {
            ret = out_->write(value.data(), len);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write string data failed");
        }
        return *this;
    }

    Serializer &operator<<(const char *value) {
        if (value == nullptr) {
            throw RuntimeException(pluginName_ + "Serializer cannot write null pointer as string");
        }
        return *this << string(value);
    }

    // Write c++ STL object
    Serializer &operator<<(const vector<int> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<int> size failed");
        for (int i = 0; i < size; ++i) {
            ret = out_->write(value[i]);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<int> element failed");
        }
        return *this;
    }
    Serializer &operator<<(const vector<long long> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<long long> size failed");
        for (int i = 0; i < size; ++i) {
            ret = out_->write(value[i]);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<long long> element failed");
        }
        return *this;
    }
#ifdef __linux__
    Serializer &operator<<(const vector<int64_t> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<int64_t> size failed");
        for (int i = 0; i < size; ++i) {
            ret = out_->write(static_cast<long long>(value[i]));
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<int64_t> element failed");
        }
        return *this;
    }
#endif
    Serializer &operator<<(const vector<double> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<double> size failed");
        for (int i = 0; i < size; ++i) {
            ret = out_->write(value[i]);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<double> element failed");
        }
        return *this;
    }
    Serializer &operator<<(const vector<string> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<string> size failed");
        for (int i = 0; i < size; ++i) {
            *this << value[i];  // Reuse string serialization
        }
        return *this;
    }
    Serializer &operator<<(const vector<DATA_TYPE> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<DATA_TYPE> size failed");
        for (int i = 0; i < size; ++i) {
            ret = out_->write(static_cast<int>(value[i]));
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write vector<DATA_TYPE> element failed");
        }
        return *this;
    }
    Serializer &operator<<(const unordered_set<int> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write unordered_set<int> size failed");
        for (const auto &item : value) {
            ret = out_->write(item);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write unordered_set<int> element failed");
        }
        return *this;
    }
    Serializer &operator<<(const unordered_set<long long> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write unordered_set<long long> size failed");
        for (const auto &item : value) {
            ret = out_->write(item);
            if (ret != OK)
                throw RuntimeException(pluginName_ + "Serializer write unordered_set<long long> element failed");
        }
        return *this;
    }
    Serializer &operator<<(const unordered_set<string> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write unordered_set<string> size failed");
        for (const auto &item : value) {
            *this << item;  // Reuse string serialization
        }
        return *this;
    }
    Serializer &operator<<(const unordered_map<string, int> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write unordered_map<string,int> size failed");
        for (const auto &it : value) {
            *this << it.first << it.second;
        }
        return *this;
    }
    Serializer &operator<<(const unordered_map<string, double> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK)
            throw RuntimeException(pluginName_ + "Serializer write unordered_map<string,double> size failed");
        for (const auto &it : value) {
            *this << it.first << it.second;
        }
        return *this;
    }
    Serializer &operator<<(const unordered_map<long long, long long> &value) {
        int size = static_cast<int>(value.size());
        IO_ERR ret = out_->write(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write unordered_map<ll,ll> size failed");
        for (const auto &it : value) {
            *this << it.first << it.second;
        }
        return *this;
    }

    // Write DDB object
    Serializer &operator<<(const VectorSP &value) {
        ddbSerialize(value);
        return *this;
    }
    Serializer &operator<<(const DictionarySP &value) {
        ddbSerialize(value);
        return *this;
    }
    Serializer &operator<<(const TableSP &value) {
        ddbSerialize(value);
        return *this;
    }
    Serializer &operator<<(const ConstantSP &value) {
        ddbSerialize(value);
        return *this;
    }

    Serializer &operator<<(const std::vector<ConstantSP> &value) {
        int size = value.size();
        *this << size;
        for (int i = 0; i < size; ++i) {
            ddbSerialize(value[i]);
        }
        return *this;
    }

    // Read primitive types
    Serializer &operator>>(bool &value) {
        IO_ERR ret = in_->readBool(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read bool failed");
        return *this;
    }
    Serializer &operator>>(char &value) {
        IO_ERR ret = in_->readChar(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read char failed");
        return *this;
    }
    Serializer &operator>>(short &value) {
        IO_ERR ret = in_->readShort(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read short failed");
        return *this;
    }
    Serializer &operator>>(int &value) {
        IO_ERR ret = in_->readInt(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read int failed");
        return *this;
    }
    Serializer &operator>>(long long &value) {
        IO_ERR ret = in_->readLong(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read long long failed");
        return *this;
    }
    Serializer &operator>>(uint32_t &value) {
        long long tmpValue;
        IO_ERR ret = in_->readLong(tmpValue);
        value = tmpValue;
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read uint32_t failed");
        return *this;
    }
#ifdef __linux__
    Serializer &operator>>(int64_t &value) {
        long long tmpValue;
        IO_ERR ret = in_->readLong(tmpValue);
        value = tmpValue;
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read int64_t failed");
        return *this;
    }
#endif
    Serializer &operator>>(size_t &value) {
        long long tmpValue;
        IO_ERR ret = in_->readLong(tmpValue);
        value = tmpValue;
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read size_t failed");
        return *this;
    }
    Serializer &operator>>(float &value) {
        IO_ERR ret = in_->readFloat(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read float failed");
        return *this;
    }
    Serializer &operator>>(double &value) {
        IO_ERR ret = in_->readDouble(value);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read double failed");
        return *this;
    }

    // Read string with length prefix
    Serializer &operator>>(string &value) {
        int len;
        IO_ERR ret = in_->readInt(len);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read string length failed");
        if (len < 0) throw RuntimeException(pluginName_ + "Serializer read invalid string length");
        if (len == 0) {
            value.clear();
            return *this;
        }
        value.resize(len);
        size_t actualLen;
        ret = in_->readBytes(&value[0], len, actualLen);
        if (ret != OK || actualLen != static_cast<size_t>(len)) {
            throw RuntimeException(pluginName_ + "Serializer read string data failed");
        }
        return *this;
    }

    // Read c++ STL object
    Serializer &operator>>(vector<int> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<int> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid vector<int> size");
        value.resize(size);
        for (int i = 0; i < size; ++i) {
            ret = in_->readInt(value[i]);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<int> element failed");
        }
        return *this;
    }
    Serializer &operator>>(vector<long long> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<long long> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid vector<long long> size");
        value.resize(size);
        for (int i = 0; i < size; ++i) {
            ret = in_->readLong(value[i]);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<long long> element failed");
        }
        return *this;
    }
#ifdef __linux__
    Serializer &operator>>(vector<int64_t> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<int64_t> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid vector<int64_t> size");
        value.resize(size);
        for (int i = 0; i < size; ++i) {
            long long tmpValue;
            ret = in_->readLong(tmpValue);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<int64_t> element failed");
            value[i] = tmpValue;
        }
        return *this;
    }
#endif
    Serializer &operator>>(vector<double> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<double> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid vector<double> size");
        value.resize(size);
        for (int i = 0; i < size; ++i) {
            ret = in_->readDouble(value[i]);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<double> element failed");
        }
        return *this;
    }
    Serializer &operator>>(vector<string> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<string> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid vector<string> size");
        value.resize(size);
        for (int i = 0; i < size; ++i) {
            *this >> value[i];  // Reuse string deserialization
        }
        return *this;
    }
    Serializer &operator>>(vector<DATA_TYPE> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<DATA_TYPE> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid vector<DATA_TYPE> size");
        value.resize(size);
        for (int i = 0; i < size; ++i) {
            int temp;
            ret = in_->readInt(temp);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read vector<DATA_TYPE> element failed");
            value[i] = static_cast<DATA_TYPE>(temp);
        }
        return *this;
    }
    Serializer &operator>>(unordered_set<int> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read unordered_set<int> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid unordered_set<int> size");
        value.clear();
        for (int i = 0; i < size; ++i) {
            int item;
            ret = in_->readInt(item);
            if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read unordered_set<int> element failed");
            value.insert(item);
        }
        return *this;
    }
    Serializer &operator>>(unordered_set<long long> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read unordered_set<long long> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid unordered_set<long long> size");
        value.clear();
        for (int i = 0; i < size; ++i) {
            long long item;
            ret = in_->readLong(item);
            if (ret != OK)
                throw RuntimeException(pluginName_ + "Serializer read unordered_set<long long> element failed");
            value.insert(item);
        }
        return *this;
    }
    Serializer &operator>>(unordered_set<string> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read unordered_set<string> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid unordered_set<string> size");
        value.clear();
        for (int i = 0; i < size; ++i) {
            string item;
            *this >> item;
            value.insert(item);
        }
        return *this;
    }
    Serializer &operator>>(unordered_map<string, int> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read unordered_map<string,int> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid unordered_map<string,int> size");
        value.clear();
        for (int i = 0; i < size; ++i) {
            string key;
            int val;
            *this >> key >> val;
            value[key] = val;
        }
        return *this;
    }
    Serializer &operator>>(unordered_map<string, double> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read unordered_map<string,double> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid unordered_map<string,double> size");
        value.clear();
        for (int i = 0; i < size; ++i) {
            string key;
            double val;
            *this >> key >> val;
            value[key] = val;
        }
        return *this;
    }
    Serializer &operator>>(unordered_map<long long, long long> &value) {
        int size;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read unordered_map<ll,ll> size failed");
        if (size < 0) throw RuntimeException(pluginName_ + "Serializer read invalid unordered_map<ll,ll> size");
        value.clear();
        for (int i = 0; i < size; ++i) {
            long long key, val;
            *this >> key >> val;
            value[key] = val;
        }
        return *this;
    }

    // Read DDB object
    Serializer &operator>>(VectorSP &value) {
        value = ddbDeserialize();
        return *this;
    }
    Serializer &operator>>(DictionarySP &value) {
        value = ddbDeserialize();
        return *this;
    }
    Serializer &operator>>(TableSP &value) {
        value = ddbDeserialize();
        return *this;
    }
    Serializer &operator>>(ConstantSP &value) {
        value = ddbDeserialize();
        return *this;
    }
    Serializer &operator>>(std::vector<ConstantSP> &value) {
        int size = 0;
        IO_ERR ret = in_->readInt(size);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read std::vector<ConstantSP> size failed");
        value.resize(size);
        for (int i = 0; i < size; ++i) {
            value[i] = ddbDeserialize();
        }
        return *this;
    }

    // Extension mechanism: allow users to specialize SerializerHelper for custom types
    template <typename T>
    struct SerializerHelper;
    // Generic write for custom types (uses SerializerHelper specialization)
    template <typename T>
    Serializer &writeCustom(const T &value) {
        SerializerHelper<T>::write(*this, value);
        return *this;
    }
    // Generic read for custom types (uses SerializerHelper specialization)
    template <typename T>
    Serializer &readCustom(T &value) {
        SerializerHelper<T>::read(*this, value);
        return *this;
    }

    template <typename T>
    Serializer &writeEnum(const T &value) {
        int intValue = static_cast<int>(value);
        return *this << intValue;
    }

    template <typename T>
    Serializer &readEnum(T &value) {
        int intValue = 0;
        *this >> intValue;
        value = static_cast<T>(intValue);
        return *this;
    }

    template <typename T>
    Serializer &writeEnumVector(const std::vector<T> &value) {
        int size = value.size();
        *this << size;
        for (auto &it : value) {
            *this << static_cast<T>(it);
        }
        return *this;
    }

    template <typename T>
    Serializer &readEnumVector(std::vector<T> &value) {
        int size = 0;
        *this >> size;
        value.resize(size);
        int intValue = 0;
        for (int i = 0; i < size; ++i) {
            *this >> intValue;
            value[i] = static_cast<T>(intValue);
        }
        return *this;
    }

    Serializer &readBuffer(char *buffer, size_t length) {
        IO_ERR ret = in_->read(buffer, length);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer read buffer failed");
        return *this;
    }

    Serializer &writeBuffer(const char *buffer, size_t length) {
        IO_ERR ret = out_->write(buffer, length);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serializer write buffer failed");
        return *this;
    }

    // Function write and read
    void writeFunction(Heap *heap, const ConstantSP &func) { funcMarshal(heap, func); }
    void readFunction(Heap *heap, FunctionDefSP &func) { func = funcUnmarshal(heap); }

  private:
    void readFromFile(const string &filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw RuntimeException(pluginName_ + "Serialize failed: failed to open file for reading: " + filePath);
        }
        auto size = file.tellg();
        fileBuffer_.resize(size);
        file.seekg(0, std::ios::beg);
        file.read(fileBuffer_.data(), size);
        if (!file.good()) {
            throw RuntimeException(pluginName_ + "Serialize failed: failed to read data from file: " + filePath);
        }
        file.close();

        in_ = new DataInputStream(fileBuffer_.data(), fileBuffer_.size());
    }

    void ddbSerialize(const ConstantSP &data) {
        if (data->containNotMarshallableObject()) {
            throw RuntimeException(pluginName_ + "Serialize failed: there is something can't be marshallable.");
        }

        IO_ERR ret;
        ConstantMarshalFactory marshallFactory(out_);
        ConstantMarshal *marshall = marshallFactory.getConstantMarshal(data->getForm());
        marshall->start(data, true, ret);
        if (ret != OK) {
            throw RuntimeException(pluginName_ + "Serialize failed: failed to serialize object.");
        }
    }

    ConstantSP ddbDeserialize() {
        short flag;
        IO_ERR ret;
        ret = in_->readShort(flag);
        auto data_form = static_cast<DATA_FORM>(flag >> 8);
        ConstantUnmarshalFactory factory(in_, nullptr);
        ConstantUnmarshal *unmarshall = factory.getConstantUnmarshal(data_form);
        if (unmarshall == nullptr) {
            throw RuntimeException(pluginName_ + "Serialize failed: failed to deserialize object.");
        }
        if (!unmarshall->start(flag, true, ret)) {
            unmarshall->reset();
            throw RuntimeException(pluginName_ +
                                   "Serialize failed: failed to deserialize object: " + std::to_string(ret) + ".");
        }

        ConstantSP result = unmarshall->getConstant();
        return result;
    }

    void funcMarshal(Heap *heap, const ConstantSP &arg) {
        IO_ERR ret = OK;
        short flag = (arg->getForm() << 8) + arg->getType();
        ret = out_->write(flag);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serialize failed: failed to serialize function.");

        const CodeMarshalSP marshal = new CodeMarshal(heap, out_);
        marshal->marshal(nullptr, 0, arg, false, ret);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serialize failed: failed to serialize function.");
    }

    ConstantSP funcUnmarshal(Heap *heap) {
        IO_ERR ret = OK;
        short flag = 0;
        ret = in_->readShort(flag);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serialize failed: failed to deserialize function.");

        ConstantUnmarshalSP unmarshal;
        unmarshal = new CodeUnmarshal(in_, heap->currentSession());
        unmarshal->start(flag, false, ret);
        if (ret != OK) throw RuntimeException(pluginName_ + "Serialize failed: failed to deserialize function.");
        ConstantSP arg = unmarshal->getConstant();
        ASSERT(dynamic_cast<MetaCode *>(arg.get()));  // CodeUnmarshal definitely wraps a MetaCode on the object
        ObjectSP obj = static_cast<MetaCode *>(arg.get())->getCode();
        if (obj.isNull() || !obj->isConstant())
            throw RuntimeException(pluginName_ + "Serialize failed: failed to deserialize function.");

        ConstantSP result = obj;
        if (result->getType() != DATA_TYPE::DT_FUNCTIONDEF)
            throw RuntimeException(pluginName_ + "Serialize failed: failed to deserialize function.");
        return result;
    }

  private:
    DataOutputStreamSP out_;
    DataInputStreamSP in_;
    vector<char> fileBuffer_;
    string pluginName_;
};

class Persistence {
  public:
    Persistence(const string &prefix, const string &fileName, const string &directory, const string &engineName,
                bool forWriting)
        : prefix_(prefix),
          fileName_(fileName),
          snapshotPath_(directory + "/" + engineName + "_" + fileName_ + ".snapshot"),
          tmpPath_(directory + "/" + engineName + "_" + fileName_ + ".tmp"),
          bakPath_(directory + "/" + engineName + "_" + fileName_ + ".bak"),
          isWriteMode_(forWriting) {
        if (fileExists(tmpPath_)) {
            std::remove(tmpPath_.c_str());
        }

        if (!forWriting) {
            if (!fileExists(snapshotPath_)) {
                throw RuntimeException(prefix_ + "Snapshot file does not exist: " + snapshotPath_);
            }
            serializer_ = Serializer(snapshotPath_, false);
        } else {
            if (fileExists(snapshotPath_)) {
                if (fileExists(bakPath_)) {
                    std::remove(bakPath_.c_str());
                }
                if (std::rename(snapshotPath_.c_str(), bakPath_.c_str()) != 0) {
                    throw RuntimeException(prefix_ + "Failed to backup existing snapshot: " + snapshotPath_ + " -> " +
                                           bakPath_);
                }
            }
            serializer_ = Serializer(tmpPath_, true);
        }
    }
    virtual ~Persistence() = default;

    Serializer &getSerializer() { return serializer_; }

    virtual void commit() {
        if (!isWriteMode_) throw RuntimeException(prefix_ + "Cannot commit in read mode.");
        serializer_.close();

        if (std::rename(tmpPath_.c_str(), snapshotPath_.c_str()) != 0) {
            throw RuntimeException(prefix_ + "Failed to rename temporary file to snapshot: " + tmpPath_ + " -> " +
                                   snapshotPath_);
        }
    }

    static bool fileExists(const string &path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }

  protected:
    string prefix_;
    string fileName_;
    string snapshotPath_;
    string tmpPath_;
    string bakPath_;
    bool isWriteMode_{false};
    Serializer serializer_;
};

}  // namespace ddb

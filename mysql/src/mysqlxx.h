#ifndef MYSQLXX_H_
#define MYSQLXX_H_

#include <climits>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#define MYSQLXX_DEFAULT_TIMEOUT 60
#define MYSQLXX_DEFAULT_RW_TIMEOUT 1800
/// Disable LOAD DATA LOCAL INFILE because it is insecure
#define MYSQLXX_DEFAULT_ENABLE_LOCAL_INFILE false

#if defined(_MSC_VER)
#define likely(x) (x)
#define unlikely(x) (x)
#else
#define likely(x) (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))
#endif

/** exp10 from GNU libm fails to give precise result for integer arguments.
 * For example, exp10(3) gives 1000.0000000000001
 *  despite the fact that 1000 is exactly representable in double and float.
 * Better to always use implementation from MUSL.
 *
 * Note: the function names are different to avoid confusion with symbols from
 * the system libm.
 */

double preciseExp10(double x) noexcept;
double precisePow10(double x) noexcept;
float preciseExp10f(float x) noexcept;
float precisePow10f(float x) noexcept;

struct st_mysql;
using MYSQL = st_mysql;

struct st_mysql_res;
using MYSQL_RES = st_mysql_res;

using MYSQL_ROW = char **;

struct st_mysql_field;
using MYSQL_FIELD = st_mysql_field;

namespace mysqlxx {
using UInt64 = uint64_t;
using Int64 = int64_t;
using UInt32 = uint32_t;
using Int32 = int32_t;

using MYSQL_LENGTH = unsigned long;
using MYSQL_LENGTHS = MYSQL_LENGTH *;
using MYSQL_FIELDS = MYSQL_FIELD *;

// copied from mariadb connector c
// DolphinDB and mysql.h headers have name conflict
// !? must ensure it's the same as mysql.h
enum enum_field_types {
    MYSQL_TYPE_DECIMAL,    // variable !?, FIXME: check this type
    MYSQL_TYPE_TINY,       // 1 byte
    MYSQL_TYPE_SHORT,      // 2 bytes
    MYSQL_TYPE_LONG,       // 4
    MYSQL_TYPE_FLOAT,      // 4
    MYSQL_TYPE_DOUBLE,     // 8
    MYSQL_TYPE_NULL,       // !?
    MYSQL_TYPE_TIMESTAMP,  // !?
    MYSQL_TYPE_LONGLONG,   // 8
    MYSQL_TYPE_INT24,      // 3
    MYSQL_TYPE_DATE,
    MYSQL_TYPE_TIME,
    MYSQL_TYPE_DATETIME,
    MYSQL_TYPE_YEAR,
    MYSQL_TYPE_NEWDATE,
    MYSQL_TYPE_VARCHAR,
    MYSQL_TYPE_BIT,
    MYSQL_TYPE_TIMESTAMP2,  // only on server side
    MYSQL_TYPE_DATETIME2,   // only on server side
    MYSQL_TYPE_TIME2,       // only on server side
    MYSQL_TYPE_JSON = 245,
    MYSQL_TYPE_NEWDECIMAL = 246,
    MYSQL_TYPE_ENUM = 247,
    MYSQL_TYPE_SET = 248,
    MYSQL_TYPE_TINY_BLOB = 249,
    MYSQL_TYPE_MEDIUM_BLOB = 250,
    MYSQL_TYPE_LONG_BLOB = 251,
    MYSQL_TYPE_BLOB = 252,
    MYSQL_TYPE_VAR_STRING = 253,
    MYSQL_TYPE_STRING = 254,
    MYSQL_TYPE_GEOMETRY = 255,
    MAX_NO_FIELD_TYPES
};

class Connection;
class Query;
class ResultBase;

// Value
class Value {
   public:
    Value(const char *data_, size_t length_, const ResultBase *res_) : m_data(data_), m_length(length_), res(res_) {}

    /// Get the value of bool.
    bool getBool() const {
        if (unlikely(isNull())) throwException("Value is NULL");

        return m_length > 0 && m_data[0] != '0';
    }

    /// Get an unsigned integer.
    UInt64 getUInt() const {
        if (unlikely(isNull())) throwException("Value is NULL");

        return readUIntText(m_data, m_length);
        ;
    }

    Int64 getInt() const {
        // return getIntOrDateTime();
        return getIntImpl();
    }

    /// Get a floating point number.
    double getDouble() const {
        if (unlikely(isNull())) throwException("Value is NULL");

        return readFloatText(m_data, m_length);
    }

    /// Get the string.
    std::string getString() const {
        if (unlikely(isNull())) throwException("Value is NULL");

        return std::string(m_data, m_length);
    }

    /// Is NULL.
    bool isNull() const { return m_data == nullptr; }

    /// For compatibility (use isNull () method instead)
    bool is_null() const { return isNull(); }

    template <typename T>
    T get() const;

    template <typename T>
    operator T() const {
        return get<T>();
    }

    const char *data() const { return m_data; }

    size_t length() const { return m_length; }

    size_t size() const { return m_length; }

    bool empty() const { return 0 == m_length; }

   private:
    const char *m_data;
    size_t m_length;
    const ResultBase *res;

    bool checkDateTime() const { return (m_length == 10 || m_length == 19) && m_data[4] == '-' && m_data[7] == '-'; }

    Int64 getIntImpl() const {
        return readIntText(m_data, m_length);
        ;
    }

    /// Read an unsigned integer in a simple format from a non-0-terminated
    /// string.
    UInt64 readUIntText(const char *buf, size_t length) const;

    /// Read a signed integer in a simple format from a non-0-terminated string.
    Int64 readIntText(const char *buf, size_t length) const;

    /// Read a floating-point number in a simple format, with rough rounding,
    /// from a non-0-terminated string.
    double readFloatText(const char *buf, size_t length) const;

    /// Throw an exception with detailed information
    void throwException(const char *text) const;
};

template <>
inline bool Value::get<bool>() const {
    return getBool();
}

template <>
inline char Value::get<char>() const {
    return getInt();
}

template <>
inline signed char Value::get<signed char>() const {
    return getInt();
}

template <>
inline unsigned char Value::get<unsigned char>() const {
    return getUInt();
}

template <>
inline short Value::get<short>() const {
    return getInt();
}

template <>
inline unsigned short Value::get<unsigned short>() const {
    return getUInt();
}

template <>
inline int Value::get<int>() const {
    return getInt();
}

template <>
inline unsigned int Value::get<unsigned int>() const {
    return getUInt();
}

template <>
inline long Value::get<long>() const {
    return getInt();
}

template <>
inline unsigned long Value::get<unsigned long>() const {
    return getUInt();
}

template <>
inline long long Value::get<long long>() const {
    return getInt();
}

template <>
inline unsigned long long Value::get<unsigned long long>() const {
    return getUInt();
}

template <>
inline float Value::get<float>() const {
    return getDouble();
}

template <>
inline double Value::get<double>() const {
    return getDouble();
}

template <>
inline std::string Value::get<std::string>() const {
    return getString();
}

/*
template<>
inline LocalDate Value::get<LocalDate>() const { return getDate(); }

template<>
inline LocalDateTime Value::get<LocalDateTime>() const { return getDateTime(); }
*/

template <typename T>
inline T Value::get() const {
    return T(*this);
}

inline std::ostream &operator<<(std::ostream &ostr, const Value &x) { return ostr.write(x.data(), x.size()); }

/** Result string.
  * Unlike mysql++,
  * is a wrapper over MYSQL_ROW (char **), refers to ResultBase, does
  * not own any data itself.
  * This means that if the result object or connection is destroyed,
  * or the next query will be specified, the Row will become incorrect.
  * When using UseQueryResult, only one resul string is stored in memory,
  * This means that after reading the next line, the previous one becomes
  * incorrect.
  */
class Row {
   private:
    /** @brief Pointer to bool data member, for use by safe bool conversion
     * operator.
     * @see http://www.artima.com/cppsource/safebool.html
     * Taken from mysql++
     */
    typedef MYSQL_ROW Row::*private_bool_type;

   public:
    /** For the possibility of delayed initialization. */
    Row() {}

    /** To create a Row, use the appropriate UseQueryResult or StoreQueryResult
     * methods. */
    Row(MYSQL_ROW row_, ResultBase *res_, MYSQL_LENGTHS lengths_) : row(row_), res(res_), lengths(lengths_) {}

    /** Get the value by index.
      * Here int is used, not unsigned, so that there is no ambiguity with the
    same method that takes const char *.   */
    Value operator[](int n) const;

    /** Get value by column name. Less efficient. */
    Value operator[](const char *name) const;

    Value operator[](const std::string &name) const { return operator[](name.c_str()); }

    /** Get the value by index. */
    Value at(size_t n) const { return operator[](n); }

    /** Number of columns. */
    size_t size() const;

    /** Is it empty? Such an object is used to mark the end of the result.
        * when using UseQueryResult. Or it means that the object is not
  initialized.     * You can use a bool conversion instead.    */
    bool empty() const { return row == nullptr; }

    /** Conversion to bool.
        * (More precisely - to the type that is converted to bool, and with
  which almost nothing can be done.)       */
    operator private_bool_type() const { return row == nullptr ? nullptr : &Row::row; }  // what the heck???
   private:
    MYSQL_ROW row = nullptr;
    ResultBase *res = nullptr;
    MYSQL_LENGTHS lengths;
};

/** Base class for UseQueryResult and StoreQueryResult.
  * Contains the general part of the implementation,
  * Refers to Connection. If you destroy Connection, then you can not
  * use ResultBase and any result.
  * You can use the object only for the result of a single query!
  * (When attempting to assign the result of the
  * following query to an object - UB.)
  */
class ResultBase {  // non-copyable
   public:
    ResultBase(MYSQL_RES *res_, Connection *conn_, const Query *query_);

    //  ResultBase(const ResultBase& other) = delete;
    //
    //  ResultBaseBase& operator=(const ResultBase& other) = delete;

    Connection *getConnection() { return conn_; }

    MYSQL_FIELDS getFields() { return fields_; }

    unsigned getNumFields() { return num_fields_; }

    MYSQL_RES *getRes() { return res_; }

    const char *nameAt(int idx);

    enum_field_types typeAt(int idx);

    bool isUnsignedAt(int idx);

    bool isEnumAt(int idx);

    unsigned long maxLengthAt(int idx);

    const Query *getQuery() const { return query_; }

    virtual ~ResultBase();

   protected:
    MYSQL_RES *res_;
    Connection *conn_;
    const Query *query_;
    MYSQL_FIELDS fields_;
    unsigned num_fields_;
};

class UseQueryResult : public ResultBase {
   public:
    UseQueryResult(MYSQL_RES *res_, Connection *conn_, const Query *query_);

    Row fetch();

    Row fetch_row() { return fetch(); }
};

class Query : public std::ostream {
   public:
    Query(Connection *conn, const std::string &query_string = "");

    Query(const Query &);

    Query &operator=(const Query &);

    ~Query();

    void reset();

    void execute();

    UseQueryResult use();

    std::string str() const { return query_buf.str(); }

    // private:
    std::stringbuf query_buf;
    Connection *conn;
};

inline std::ostream &operator<<(std::ostream &ostr, const Query &query) { return ostr << query.rdbuf(); }

class Exception : public std::exception {
   public:
    Exception(const std::string &msg, int code = INT_MAX);

    Exception(const Exception &rhs) noexcept;  // nothrow copy constructible

    ~Exception() noexcept override;

    const char *what() noexcept;

    virtual const char *name() noexcept;

    std::string displayText() noexcept;

   private:
    std::string msg_ = "";
    int code_ = INT_MAX;
};

struct ConnectionFailed : public Exception {
    ConnectionFailed(const std::string &msg, int code = INT_MAX) noexcept : Exception(msg, code) {}

    const char *name() noexcept { return "ConnectionFailed"; }
};

struct BadQuery : public Exception {
    BadQuery(const std::string &msg, int code = INT_MAX) noexcept : Exception(msg, code) {}

    const char *name() noexcept { return "BadQuery"; }
};

struct CannotParseValue : public Exception {
    CannotParseValue(const std::string &msg, int code = INT_MAX) noexcept : Exception(msg, code) {}

    const char *name() noexcept { return "CannotParseValue"; }
};

std::string errorMessage(MYSQL *driver);

/// For internal need of library.
void checkError(MYSQL *driver);
void onError(MYSQL *driver);

// Connection
class LibrarySingleton final {
   private:
    LibrarySingleton();

   public:
    static LibrarySingleton &instance() {
        static LibrarySingleton instance;
        return instance;
    }
};

class Connection {
   public:
    Connection();

    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;

    //  Connection(const Connection &other) {
    //    std::cout << "copy connection" << std::endl;
    //  }
    //
    //  Connection &operator=(const Connection &other) {
    //    std::cout << "assign connection" << std::endl;
    //    return *this;
    //  }

    virtual ~Connection();

    Connection(const char *db,
               const char *server,
               const char *user = nullptr,
               const char *password = nullptr,
               unsigned port = 3306,
               const char *socket = "",
               const char *ssl_ca = "",
               const char *ssl_cert = "",
               const char *ssl_key = "",
               unsigned timeout = MYSQLXX_DEFAULT_TIMEOUT,
               unsigned rw_timeout = MYSQLXX_DEFAULT_RW_TIMEOUT,
               bool enable_local_infile = MYSQLXX_DEFAULT_ENABLE_LOCAL_INFILE);

    // delayed connection
    void connect(const char *db,
                 const char *server,
                 const char *user = nullptr,
                 const char *password = nullptr,
                 unsigned port = 0,
                 const char *socket = "",
                 const char *ssl_ca = "",
                 const char *ssl_cert = "",
                 const char *ssl_key = "",
                 unsigned timeout = MYSQLXX_DEFAULT_TIMEOUT,
                 unsigned rw_timeout = MYSQLXX_DEFAULT_RW_TIMEOUT,
                 bool enable_local_infile = MYSQLXX_DEFAULT_ENABLE_LOCAL_INFILE);

    void disconnect();

    bool connected();

    bool ping();

    Query query(const std::string &str = "");

    MYSQL *getDriver();

   private:
    bool connected_ = false;
    bool initialized_ = false;
    std::shared_ptr<MYSQL> driver;
};

}  // namespace mysqlxx

#endif
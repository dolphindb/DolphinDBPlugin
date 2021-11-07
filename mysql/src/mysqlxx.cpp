#include "mysqlxx.h"
#include <math.h>
#include <mysql.h>
#include <stdint.h>
#include <limits>

namespace mysqlxx {

static inline const char *ifNotEmpty(const char *s) { return s && *s ? s : nullptr; }

LibrarySingleton::LibrarySingleton() {
    if (mysql_library_init(0, nullptr, nullptr)) throw Exception("Cannot initialize MySQL library.");
}

Connection::Connection() : driver(std::make_shared<MYSQL>()) { LibrarySingleton::instance(); }

Connection::~Connection() {
    disconnect();
    mysql_thread_end();
    //  std::cout << "Connection destructed." << std::endl;
}

Connection::Connection(const char *db,
                       const char *server,
                       const char *user,
                       const char *password,
                       unsigned int port,
                       const char *socket,
                       const char *ssl_ca,
                       const char *ssl_cert,
                       const char *ssl_key,
                       unsigned int timeout,
                       unsigned int rw_timeout,
                       bool enable_local_infile)
    : Connection() {
    connect(db, server, user, password, port, socket, ssl_ca, ssl_cert, ssl_key, timeout, rw_timeout, enable_local_infile);
}

void Connection::connect(const char *db,
                         const char *server,
                         const char *user,
                         const char *password,
                         unsigned port,
                         const char *socket,
                         const char *ssl_ca,
                         const char *ssl_cert,
                         const char *ssl_key,
                         unsigned timeout,
                         unsigned rw_timeout,
                         bool enable_local_infile) {
    if (connected_) disconnect();

    if (!mysql_init(driver.get())) throw ConnectionFailed(errorMessage(driver.get()), mysql_errno(driver.get()));
    initialized_ = true;

    /// Set timeouts.
    if (mysql_options(driver.get(), MYSQL_OPT_CONNECT_TIMEOUT, &timeout))
        throw ConnectionFailed(errorMessage(driver.get()), mysql_errno(driver.get()));

    if (mysql_options(driver.get(), MYSQL_OPT_READ_TIMEOUT, &rw_timeout))
        throw ConnectionFailed(errorMessage(driver.get()), mysql_errno(driver.get()));

    if (mysql_options(driver.get(), MYSQL_OPT_WRITE_TIMEOUT, &rw_timeout))
        throw ConnectionFailed(errorMessage(driver.get()), mysql_errno(driver.get()));

    /// Disable LOAD DATA LOCAL INFILE because it is insecure if necessary.
    auto enable_local_infile_arg = static_cast<unsigned>(enable_local_infile);
    if (mysql_options(driver.get(), MYSQL_OPT_LOCAL_INFILE, &enable_local_infile_arg))
        throw ConnectionFailed(errorMessage(driver.get()), mysql_errno(driver.get()));

    /// Specifies particular ssl key and certificate if it needs
    if (mysql_ssl_set(driver.get(), ifNotEmpty(ssl_key), ifNotEmpty(ssl_cert), ifNotEmpty(ssl_ca), nullptr, nullptr))
        throw ConnectionFailed(errorMessage(driver.get()), mysql_errno(driver.get()));

    if (!mysql_real_connect(driver.get(), server, user, password, db, port, ifNotEmpty(socket), driver->client_flag))
        throw ConnectionFailed(errorMessage(driver.get()), mysql_errno(driver.get()));

    /// Sets UTF-8 as default encoding.
    if (mysql_set_character_set(driver.get(), "UTF8")) throw ConnectionFailed(errorMessage(driver.get()), mysql_errno(driver.get()));

    /// Enables auto-reconnect.
    my_bool reconnect = true;
    if (mysql_options(driver.get(), MYSQL_OPT_RECONNECT, reinterpret_cast<const char *>(&reconnect)))
        throw ConnectionFailed(errorMessage(driver.get()), mysql_errno(driver.get()));

    connected_ = true;
}

void Connection::disconnect() {
    if (!initialized_) return;

    mysql_close(driver.get());
    memset(driver.get(), 0, sizeof(*driver));

    initialized_ = false;
    connected_ = false;
}

bool Connection::connected() { return connected_; }

bool Connection::ping() { return connected_ && !mysql_ping(driver.get()); }

Query Connection::query(const std::string &str) { return {this, str}; }

MYSQL *Connection::getDriver() { return driver.get(); }

// Exception
Exception::Exception(const std::string &msg, int code) : msg_(msg), code_(code) {}

Exception::Exception(const Exception &rhs) noexcept : msg_(rhs.msg_), code_(rhs.code_) {}

Exception::~Exception() noexcept = default;

const char *Exception::what() noexcept { return name(); }

const char *Exception::name() noexcept { return "Exception"; }

std::string Exception::displayText() noexcept {
    // std::string txt = name();
    std::string txt;
    if (!msg_.empty()) {
        txt.append(": ");
        txt.append(msg_);
    }
    if (code_ != INT_MAX) {
        txt.append(", with errno: ");
        txt.append(std::to_string(code_));
    }
    return txt;
}

// adopted from clickhouse
std::string errorMessage(MYSQL *driver) {
    std::stringstream res;
    res << mysql_error(driver) << " (" << (driver->host ? driver->host : "(nullptr)") << ":" << driver->port << ")";
    return res.str();
}

/// Для внутренних нужд библиотеки.
void checkError(MYSQL *driver) {
    unsigned num = mysql_errno(driver);

    if (num) throw Exception(errorMessage(driver), num);
}

/// Для внутренних нужд библиотеки.
void onError(MYSQL *driver) { throw Exception(errorMessage(driver), mysql_errno(driver)); }

// Query
Query::Query(mysqlxx::Connection *conn_, const std::string &query_string) : conn(conn_) {
    mysql_thread_init();

    init(&query_buf);

    if (!query_string.empty()) {
        query_buf.str(query_string);
        seekp(0, std::ios::end);
    }

    imbue(std::locale::classic());
}

Query::Query(const Query &other) : std::ostream(0), conn(other.conn) {
    mysql_thread_init();

    init(&query_buf);
    imbue(std::locale::classic());

    *this << other.str();
}

Query &Query::operator=(const Query &other) {
    conn = other.conn;

    seekp(0);
    clear();  // clear state
    *this << other.str();

    return *this;
}

Query::~Query() { mysql_thread_end(); }

void Query::reset() {
    seekp(0);
    clear();
    query_buf.str("");
}

void Query::execute() {
    conn->getDriver();
    std::string query_string = query_buf.str();
    if (mysql_real_query(conn->getDriver(), query_string.data(), query_string.size()))
        throw BadQuery(errorMessage(conn->getDriver()), mysql_errno(conn->getDriver()));
}

UseQueryResult Query::use() {
    execute();
    auto res = mysql_use_result(conn->getDriver());
    if (!res) onError(conn->getDriver());

    return {res, conn, this};
}

// Value
UInt64 Value::readUIntText(const char *buf, size_t length) const {
    UInt64 x = 0;
    const char *end = buf + length;

    while (buf != end) {
        switch (*buf) {
            case '+':
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                x *= 10;
                x += *buf - '0';
                break;
            default:
                throwException("Cannot parse unsigned integer");
        }
        ++buf;
    }

    return x;
}

Int64 Value::readIntText(const char *buf, size_t length) const {
    bool negative = false;
    UInt64 x = 0;
    const char *end = buf + length;

    while (buf != end) {
        switch (*buf) {
            case '+':
                break;
            case '-':
                negative = true;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                x *= 10;
                x += *buf - '0';
                break;
            default:
                throwException("Cannot parse signed integer");
        }
        ++buf;
    }

    return negative ? -x : x;
}

double Value::readFloatText(const char *buf, size_t length) const {
    bool negative = false;
    double x = 0;
    bool after_point = false;
    double power_of_ten = 1;
    const char *end = buf + length;

    while (buf != end) {
        switch (*buf) {
            case '+':
                break;
            case '-':
                negative = true;
                break;
            case '.':
                after_point = true;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (after_point) {
                    power_of_ten /= 10;
                    x += (*buf - '0') * power_of_ten;
                } else {
                    x *= 10;
                    x += *buf - '0';
                }
                break;
            case 'e':
            case 'E': {
                ++buf;
                Int32 exponent = readIntText(buf, end - buf);
                x *= preciseExp10(exponent);
                if (negative) x = -x;
                return x;
            }
            case 'i':
            case 'I':
                x = std::numeric_limits<double>::infinity();
                if (negative) x = -x;
                return x;
            case 'n':
            case 'N':
                x = std::numeric_limits<double>::quiet_NaN();
                return x;
            default:
                throwException("Cannot parse floating point number");
        }
        ++buf;
    }
    if (negative) x = -x;

    return x;
}

void Value::throwException(const char *text) const {
    static constexpr size_t MYSQLXX_QUERY_PREVIEW_LENGTH = 1000;

    std::stringstream info;
    info << text;

    if (!isNull()) {
        info << ": ";
        info.write(m_data, m_length);
    }

    if (res && res->getQuery()) info << ", query: " << res->getQuery()->str().substr(0, MYSQLXX_QUERY_PREVIEW_LENGTH);

    throw CannotParseValue(info.str());
}

// Use Result

UseQueryResult::UseQueryResult(MYSQL_RES *res_, Connection *conn_, const Query *query_) : ResultBase(res_, conn_, query_) {}

Row UseQueryResult::fetch() {
    MYSQL_ROW row = mysql_fetch_row(res_);
    if (!row) checkError(conn_->getDriver());

    return {row, this, mysql_fetch_lengths(res_)};
}

// Row

Value Row::operator[](int n) const {
    if (unlikely(static_cast<size_t>(n) >= res->getNumFields())) throw Exception("Index of column is out of range.");
    return {row[n], lengths[n], res};
}

Value Row::operator[](const char *name) const {
    unsigned n = res->getNumFields();
    MYSQL_FIELDS fields = res->getFields();

    for (unsigned i = 0; i < n; ++i) {
        if (!strcmp(name, fields[i].name)) {
            return operator[](i);
        }
    }

    throw Exception(std::string("Unknown column, ") + name);
}

size_t Row::size() const { return res->getNumFields(); }

enum_field_types Row::getTypeAt(int n) const { return res->typeAt(n); }
// ResultBase

ResultBase::ResultBase(MYSQL_RES *res_, Connection *conn_, const Query *query_) : res_(res_), conn_(conn_), query_(query_) {
    fields_ = mysql_fetch_field(res_);
    num_fields_ = mysql_num_fields(res_);
}

ResultBase::~ResultBase() { mysql_free_result(res_); }

const char *ResultBase::nameAt(int idx) { return fields_[idx].name; }

enum_field_types ResultBase::typeAt(int idx) { return static_cast<enum_field_types>(fields_[idx].type); }

bool ResultBase::isEnumAt(int idx) { return fields_[idx].flags & ENUM_FLAG; }
bool ResultBase::isUnsignedAt(int idx) { return fields_[idx].flags & UNSIGNED_FLAG; }

unsigned long ResultBase::maxLengthAt(int idx) { return fields_[idx].length; }

}  // namespace mysqlxx

/*

https://www.musl-libc.org/
http://git.musl-libc.org/cgit/musl/tree/src/math/exp10.c

musl as a whole is licensed under the following standard MIT license:

----------------------------------------------------------------------
Copyright © 2005-2014 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
----------------------------------------------------------------------

Authors/contributors include:

Alex Dowad
Alexander Monakov
Anthony G. Basile
Arvid Picciani
Bobby Bingham
Boris Brezillon
Brent Cook
Chris Spiegel
Clément Vasseur
Daniel Micay
Denys Vlasenko
Emil Renner Berthing
Felix Fietkau
Felix Janda
Gianluca Anzolin
Hauke Mehrtens
Hiltjo Posthuma
Isaac Dunham
Jaydeep Patil
Jens Gustedt
Jeremy Huntwork
Jo-Philipp Wich
Joakim Sindholt
John Spencer
Josiah Worcester
Justin Cormack
Khem Raj
Kylie McClain
Luca Barbato
Luka Perkov
M Farkas-Dyck (Strake)
Mahesh Bodapati
Michael Forney
Natanael Copa
Nicholas J. Kain
orc
Pascal Cuoq
Petr Hosek
Pierre Carrier
Rich Felker
Richard Pennington
Shiz
sin
Solar Designer
Stefan Kristiansson
Szabolcs Nagy
Timo Teräs
Trutz Behn
Valentin Ochs
William Haddon

Portions of this software are derived from third-party works licensed
under terms compatible with the above MIT license:

The TRE regular expression implementation (src/regex/reg* and
src/regex/tre*) is Copyright © 2001-2008 Ville Laurikari and licensed
under a 2-clause BSD license (license text in the source files). The
included version has been heavily modified by Rich Felker in 2012, in
the interests of size, simplicity, and namespace cleanliness.

Much of the math library code (src/math/ * and src/complex/ *) is
Copyright © 1993,2004 Sun Microsystems or
Copyright © 2003-2011 David Schultz or
Copyright © 2003-2009 Steven G. Kargl or
Copyright © 2003-2009 Bruce D. Evans or
Copyright © 2008 Stephen L. Moshier
and labelled as such in comments in the individual source files. All
have been licensed under extremely permissive terms.

The ARM memcpy code (src/string/arm/memcpy_el.S) is Copyright © 2008
The Android Open Source Project and is licensed under a two-clause BSD
license. It was taken from Bionic libc, used on Android.

The implementation of DES for crypt (src/crypt/crypt_des.c) is
Copyright © 1994 David Burren. It is licensed under a BSD license.

The implementation of blowfish crypt (src/crypt/crypt_blowfish.c) was
originally written by Solar Designer and placed into the public
domain. The code also comes with a fallback permissive license for use
in jurisdictions that may not recognize the public domain.

The smoothsort implementation (src/stdlib/qsort.c) is Copyright © 2011
Valentin Ochs and is licensed under an MIT-style license.

The BSD PRNG implementation (src/prng/random.c) and XSI search API
(src/search/ *.c) functions are Copyright © 2011 Szabolcs Nagy and
licensed under following terms: "Permission to use, copy, modify,
and/or distribute this code for any purpose with or without fee is
hereby granted. There is no warranty."

The x86_64 port was written by Nicholas J. Kain and is licensed under
the standard MIT terms.

The mips and microblaze ports were originally written by Richard
Pennington for use in the ellcc project. The original code was adapted
by Rich Felker for build system and code conventions during upstream
integration. It is licensed under the standard MIT terms.

The mips64 port was contributed by Imagination Technologies and is
licensed under the standard MIT terms.

The powerpc port was also originally written by Richard Pennington,
and later supplemented and integrated by John Spencer. It is licensed
under the standard MIT terms.

All other files which have no copyright comments are original works
produced specifically for use as part of this library, written either
by Rich Felker, the main author of the library, or by one or more
contibutors listed above. Details on authorship of individual files
can be found in the git version control history of the project. The
omission of copyright and license comments in each file is in the
interest of source tree size.

In addition, permission is hereby granted for all public header files
(include/ * and arch/ * /bits/ *) and crt files intended to be linked into
applications (crt/ *, ldso/dlstart.c, and arch/ * /crt_arch.h) to omit
the copyright notice and permission notice otherwise required by the
license, and to use these files without any requirement of
attribution. These files include substantial contributions from:

Bobby Bingham
John Spencer
Nicholas J. Kain
Rich Felker
Richard Pennington
Stefan Kristiansson
Szabolcs Nagy

all of whom have explicitly granted such permission.

This file previously contained text expressing a belief that most of
the files covered by the above exception were sufficiently trivial not
to be subject to copyright, resulting in confusion over whether it
negated the permissions granted in the license. In the spirit of
permissive licensing, and of not having licensing issues being an
obstacle to adoption, that text has been removed.

*/

double preciseExp10(double x) noexcept {
    static const double p10[] = {1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1,
                                 1e1,   1e2,   1e3,   1e4,   1e5,   1e6,   1e7,  1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15};
    double n, y = modf(x, &n);
    union {
        double f;
        uint64_t i;
    } u = {n};
    /* fabs(n) < 16 without raising invalid on nan */
    if ((u.i >> 52 & 0x7ff) < 0x3ff + 4) {
        if (!y) return p10[(int)n + 15];
        y = exp2(3.32192809488736234787031942948939 * y);
        return y * p10[(int)n + 15];
    }
    return pow(10.0, x);
}

float preciseExp10f(float x) noexcept {
    static const float p10[] = {1e-7f, 1e-6f, 1e-5f, 1e-4f, 1e-3f, 1e-2f, 1e-1f, 1, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7};
    float n, y = modff(x, &n);
    union {
        float f;
        uint32_t i;
    } u = {n};
    /* fabsf(n) < 8 without raising invalid on nan */
    if ((u.i >> 23 & 0xff) < 0x7f + 3) {
        if (!y) return p10[(int)n + 7];
        y = exp2f(3.32192809488736234787031942948939f * y);
        return y * p10[(int)n + 7];
    }
    return exp2(3.32192809488736234787031942948939 * x);
}

double precisePow10(double x) noexcept { return preciseExp10(x); }

float precisePow10f(float x) noexcept { return preciseExp10f(x); }


#ifndef HDFS_KERBEROS_H
#define HDFS_KERBEROS_H

#include "Exceptions.h"
#include <cstddef>
#include <functional>
#include <krb5.h>
#include <com_err.h>
#include <string>
#include "HdfsWrapper.h"
#include "Util.h"

using ddb::ConstantSP;
using ddb::SmartPointer;
using ddb::RuntimeException;
using ddb::Util;
using std::vector;

class Defer {
public:
    Defer(std::function<void()> code): code(code) {}
    ~Defer() { code(); }
private:
    std::function<void()> code;
};

namespace hdfsPlugin {

static int countLifeTime(const string& lifeTime) {
    int defaultLifeTime = 3600 * 24;
    int ret = 0;
    if(lifeTime == "") {
        return defaultLifeTime;
    }
    bool dayFlag = false;
    bool hourFlag = false;
    bool minuteFlag = false;
    bool secondFlag = false;
    int current = -1;
    int sign = 1;
    for(size_t i = 0; i < lifeTime.size(); ++i) {
        char c = lifeTime[i];
        if(c == '+') {
            if(current != -1) {
                throw RuntimeException(PLUGIN_HDFS_PREFIX + "bad lifetime value " + lifeTime + ".");
            }
            sign = 1;
        } else if(c == '-') {
            if(current != -1) {
                throw RuntimeException(PLUGIN_HDFS_PREFIX + "bad lifetime value " + lifeTime + ".");
            }
            sign = -1;
        } else if(!Util::isDigit(c)) {
            if(c == 'D' || c == 'd') {
                if(dayFlag || current == -1) {
                    throw RuntimeException(PLUGIN_HDFS_PREFIX + "bad lifetime value " + lifeTime + ".");
                } else {
                    dayFlag = true;
                }
                ret += current * 3600 * 24;
                current = -1;
                sign = 1;
            } else if(c == 'H' || c == 'h') {
                if(hourFlag || current == -1) {
                    throw RuntimeException(PLUGIN_HDFS_PREFIX + "bad lifetime value " + lifeTime + ".");
                } else {
                    hourFlag = true;
                }
                ret += current * 3600;
                current = -1;
                sign = 1;
            } else if(c == 'M' || c == 'm') {
                if(minuteFlag || current == -1) {
                    throw RuntimeException(PLUGIN_HDFS_PREFIX + "bad lifetime value " + lifeTime + ".");
                } else {
                    minuteFlag = true;
                }
                ret += current * 60;
                current = -1;
                sign = 1;
            } else if(c == 'S' || c == 's') {
                if(secondFlag || current == -1) {
                    throw RuntimeException(PLUGIN_HDFS_PREFIX + "bad lifetime value " + lifeTime + ".");
                } else {
                    secondFlag = true;
                }
                ret += current;
                current = -1;
                sign = 1;
            } else {
                    throw RuntimeException(PLUGIN_HDFS_PREFIX + "bad lifetime value " + lifeTime + ".");
                return defaultLifeTime;
            }
        } else {
            if(current == -1) {
                current = 0;
            }
            current = current * 10 + sign * (c - '0');
        }
    }
    return ret;

}
static bool hdfsKInit(
    const string& keytabFile,
    const string& cacheFile,
    const string& principal,
    const string& lifeTime = ""
) {
    int kerbLifeTime = countLifeTime(lifeTime);
    try {
        bool success = false;
        krb5_error_code ret;
        krb5_get_init_creds_opt *options = NULL;
        krb5_principal krb5Principal = NULL;
        krb5_context ctx = NULL;
        krb5_keytab keytab = NULL;
        krb5_ccache cache = NULL;
        krb5_creds creds;

        // init context
        if ((ret = krb5_init_context(&ctx)))
            throw RuntimeException(PLUGIN_HDFS_PREFIX + "cannot initialize context");
        // make sure resource release
        Defer deferCtx([&]() {if (ctx) {krb5_free_context(ctx);}});

        // Get a handle for a key table
        if ((ret = krb5_kt_resolve(ctx, keytabFile.c_str(), &keytab)))
            throw RuntimeException(PLUGIN_HDFS_PREFIX + "cannot resolve keytab");
        Defer deferKeytab([&]() {if (keytab) {krb5_kt_close(ctx, keytab);}});

        // Resolve a credential cache name
        if ((ret = krb5_cc_resolve(ctx, cacheFile.c_str(), &cache)))
            throw RuntimeException(PLUGIN_HDFS_PREFIX + "cannot open/initialize kerberos cache");
        Defer deferCache([&]() {if (cache) {krb5_cc_close(ctx, cache);}});

        // Convert a string principal name to a krb5_principal structure
        if ((ret = krb5_parse_name(ctx, principal.c_str(), &krb5Principal)))
            throw RuntimeException(PLUGIN_HDFS_PREFIX + "cannot parse principal string");
        Defer deferPrincipal([&]() {if (krb5Principal) {krb5_free_principal(ctx, krb5Principal);}});

        // Allocate a new initial credential options structure
        ret = krb5_get_init_creds_opt_alloc(ctx, &options);
        if (ret) {
            throw RuntimeException(PLUGIN_HDFS_PREFIX + "cannot initialize credential options");
        }
        Defer deferOption([&]() {if (options) {krb5_get_init_creds_opt_free(ctx, options);}});


        krb5_deltat ticketLifetime = kerbLifeTime;
        // Set the ticket lifetime in initial credential options
        krb5_get_init_creds_opt_set_tkt_life(options, ticketLifetime);

        // Get initial credentials using a key table
        if ((ret = krb5_get_init_creds_keytab(ctx, &creds, krb5Principal, keytab, 0, NULL, options))) {
            throw RuntimeException(PLUGIN_HDFS_PREFIX + "cannot initialize keytab credentials [" + keytabFile
                + "] with err code " + std::to_string(ret) + ".");
        }
        // Defer deferCreds([&]() {krb5_free_creds(ctx, &creds);});

        // Initialize a credential cache
        if ((ret = krb5_cc_initialize(ctx, cache, krb5Principal))) {
            throw RuntimeException(PLUGIN_HDFS_PREFIX + "cannot initialize cache [" + cacheFile + "].");
        }

        // Store credentials in a credential cache
        if ((ret = krb5_cc_store_cred(ctx, cache, &creds))) {
            throw RuntimeException(PLUGIN_HDFS_PREFIX + "cannot store credentials[" + cacheFile + "].");
        }
        return success;
    } catch (RuntimeException &ex) {
        throw ex;
    } catch (std::exception &ex) {
        throw RuntimeException(PLUGIN_HDFS_PREFIX + ex.what());
    }
}
}  // namespace hdfsPlugin
#endif
#include "DolphinDBEverything.h"

#include "s3_config.h"

#include "ddbplugin/PluginLogger.h"
#include "ddbplugin/PluginTypeUtils.h"

#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/auth/signer/AWSAuthV4Signer.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/memory/stl/AWSString.h>

#include <limits>
#include <memory>
#include <mutex>
#include <string>

#if __cplusplus >= 201703L
#include <optional>
#else
#include "ddbplugin/FixSTL.h"
#endif

using namespace ddb; // NOLINT(google-build-using-namespace)

namespace {

std::mutex clientConfigMutex; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::unique_ptr<Aws::S3::S3ClientConfiguration> clientConfig; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

struct s3_account_options {
    std::optional<std::string> id;
    std::optional<std::string> key;
    std::optional<std::string> endpoint;
    std::optional<std::string> region;
    std::optional<bool> is_http;
    std::optional<bool> verify_ssl;
    std::optional<std::string> ca_path;
    std::optional<std::string> ca_file;
    std::optional<long> request_timeout_ms;
};

std::optional<unsigned> get_max_connections(DictionarySP config) {
    auto max_connections = config->getMember("maxConnections");
    if (max_connections->isNull()) {
        return std::nullopt;
    }
    auto value = arg_to_int64(max_connections);
    if (!value || *value <= 0 || *value > std::numeric_limits<unsigned>::max()) {
        throw IllegalArgumentException("setClientConfig", "maxConnections should be a positive integer.");
    }
    return static_cast<unsigned>(*value);
}

std::optional<long> get_request_timeout_ms(DictionarySP config, const std::string &func) {
    auto request_timeout = config->getMember("requestTimeoutMs");
    if (request_timeout->isNull()) {
        return std::nullopt;
    }
    auto value = arg_to_int64(request_timeout);
    if (!value || *value < 0 || *value > std::numeric_limits<long>::max()) {
        throw IllegalArgumentException(func, "requestTimeoutMs should be a non-negative integer.");
    }
    return static_cast<long>(*value);
}

std::optional<std::string> get_optional_string(DictionarySP config, const std::string &key, const std::string &func) {
    auto value = config->getMember(key);
    if (value->isNull()) {
        return std::nullopt;
    }
    auto parsed = arg_to_string(value);
    if (!parsed) {
        throw IllegalArgumentException(func, key + " should be a string scalar.");
    }
    return parsed;
}

std::optional<bool> get_optional_bool(DictionarySP config, const std::string &key, const std::string &func) {
    auto value = config->getMember(key);
    if (value->isNull()) {
        return std::nullopt;
    }
    if (value->getForm() != DF_SCALAR || value->getType() != DT_BOOL) {
        throw IllegalArgumentException(func, key + " should be a bool scalar.");
    }
    return static_cast<bool>(value->getBool());
}

Aws::S3::S3ClientConfiguration create_default_client_config()
{
    constexpr int request_timeout_ms{300 * 1000};
    auto config = Aws::S3::S3ClientConfiguration(Aws::Auth::GetConfigProfileName().c_str());
    // Try to fix "BadDigest" errors.
    config.checksumConfig.requestChecksumCalculation = Aws::Client::RequestChecksumCalculation::WHEN_REQUIRED;
    config.payloadSigningPolicy = Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never; // for upload speed
    config.requestTimeoutMs = request_timeout_ms;                                            // for large file upload
    config.verifySSL = false;
    return config;
}

s3_account_options read_s3_account_options(DictionarySP account) {
    s3_account_options options;
    options.id = get_optional_string(account, "id", "S3account");
    options.key = get_optional_string(account, "key", "S3account");
    if (options.id.has_value() != options.key.has_value()) {
        throw IllegalArgumentException("S3account",
                                       "s3account should have both id and key, or neither to use AWS SDK default credential provider chain");
    }
    options.endpoint = get_optional_string(account, "endpoint", "S3account");
    options.region = get_optional_string(account, "region", "S3account");
    options.is_http = get_optional_bool(account, "isHttp", "S3account");
    options.verify_ssl = get_optional_bool(account, "verifySSL", "S3account");
    options.ca_path = get_optional_string(account, "caPath", "S3account");
    options.ca_file = get_optional_string(account, "caFile", "S3account");
    options.request_timeout_ms = get_request_timeout_ms(account, "S3account");
    return options;
}

} // namespace

void initialize_s3_client_config() {
    std::lock_guard<std::mutex> lock(clientConfigMutex);
    clientConfig = std::make_unique<Aws::S3::S3ClientConfiguration>(create_default_client_config());
}

void update_s3_client_config(DictionarySP config) {
    auto max_connections = get_max_connections(config);
    auto request_timeout_ms = get_request_timeout_ms(config, "setClientConfig");
    auto verify_ssl = get_optional_bool(config, "verifySSL", "setClientConfig");

    std::lock_guard<std::mutex> lock(clientConfigMutex);
    if (max_connections) {
        clientConfig->maxConnections = *max_connections;
    }
    if (request_timeout_ms) {
        clientConfig->requestTimeoutMs = *request_timeout_ms;
    }
    if (verify_ssl) {
        clientConfig->verifySSL = *verify_ssl;
    }
}

auto get_s3_client_config() -> Aws::S3::S3ClientConfiguration {
    std::lock_guard<std::mutex> lock(clientConfigMutex);
    return *clientConfig;
}

auto configure_s3_account(DictionarySP account, const Aws::S3::S3ClientConfiguration &client_config)
    -> s3_account_config {
    auto options = read_s3_account_options(std::move(account));
    s3_account_config account_config;
    account_config.config = client_config;

    account_config.use_explicit_credentials = options.id.has_value();
    if (account_config.use_explicit_credentials) {
        account_config.credential.SetAWSAccessKeyId(Aws::String(*options.id));
        account_config.credential.SetAWSSecretKey(Aws::String(*options.key));
    }

    if (options.endpoint) {
        account_config.config.endpointOverride = Aws::String(*options.endpoint);
        if (options.is_http) {
            if (*options.is_http) {
                account_config.config.scheme = Aws::Http::Scheme::HTTP;
            }
            LOG_INFO("aws s3 scheme is http: ", *options.is_http);
        }
        account_config.config.useVirtualAddressing = false;
    }
    if (options.region) {
        account_config.config.region = Aws::String(*options.region);
    }
    const bool has_ca_config = options.ca_path.has_value() || options.ca_file.has_value();
    account_config.config.verifySSL = options.verify_ssl.value_or(account_config.config.verifySSL);
    if (has_ca_config && !account_config.config.verifySSL) {
        throw IllegalArgumentException("S3account", "caPath and caFile require verifySSL=true.");
    }
    if (options.ca_path) {
        account_config.config.caPath = Aws::String(*options.ca_path);
    }
    if (options.ca_file) {
        account_config.config.caFile = Aws::String(*options.ca_file);
    }
    if (options.request_timeout_ms) {
        account_config.config.requestTimeoutMs = *options.request_timeout_ms;
    }

    LOG_INFO("aws s3 use credential provider chain: ", !account_config.use_explicit_credentials);
    return account_config;
}

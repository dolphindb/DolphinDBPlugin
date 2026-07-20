#pragma once

#include "DolphinDBEverything.h"

#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3ClientConfiguration.h>

struct s3_account_config {
    Aws::Auth::AWSCredentials credential;
    Aws::S3::S3ClientConfiguration config;
    bool use_explicit_credentials = false;
};

void initialize_s3_client_config();
void update_s3_client_config(DictionarySP config);
auto get_s3_client_config() -> Aws::S3::S3ClientConfiguration;
auto configure_s3_account(DictionarySP account, const Aws::S3::S3ClientConfiguration &client_config)
    -> s3_account_config;

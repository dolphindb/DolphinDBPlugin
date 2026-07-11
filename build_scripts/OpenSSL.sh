#!/bin/bash

# Building OpenSSL requires perl which introduces a lot of devel packages, so we build it in a docker container.
# This is the core build config (--release is default and omitted):
# ./Configure --release no-shared no-apps no-docs no-tests no-legacy no-async
# Other useful but unavailable options:
# no-deprecated: AWS/MongoDB/MariaDB does NOT use 3.0 APIs.
# enable-ktls: required kernel version 5.10+ and SSL_set_options(SSL_OP_ENABLE_KTLS).

prefix=openssl-4.0.0

openssl_dir=/root/openssl
cat > build.sh << EOF
#!/usr/bin/env bash
set -x
export HOME=$HOME
source $HOME/jenkins_util.sh
select_toolchain $Compiler
if [ "$Compiler" == "gcc-4.8.5" ]; then
	export CFLAGS="$CFLAGS -std=gnu99"
fi
cross=""
if [ -n "$CROSS_TOOLCHAIN" ]; then
	cross="linux-aarch64"
fi

cd $openssl_dir
./Configure \$cross --prefix=$ARTIFACT_DIR/$prefix --libdir=lib64 \
	no-shared no-apps no-docs no-tests no-legacy no-async
make -j$(nproc)
make install
cd ..
rm -rf build

chown -R $(id -u):$(id -g) $ARTIFACT_DIR/$prefix
EOF

chmod +x build.sh
docker run --rm -v $(pwd):$openssl_dir -v $HOME:$HOME build_env:OpenSSL $openssl_dir/build.sh

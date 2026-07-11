#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)

cd src
autoreconf -fi
export CFLAGS="-I$openssl_dir/include $CFLAGS"
export LDFLAGS="-L$openssl_dir/lib64"
export LIBS="-ldl -lpthread"
gnu_all
cd $ARTIFACT_DIR/$prefix/lib
$AR -M << EOF
create libkrb5_combined.a
addlib libgssapi_krb5.a
addlib libk5crypto.a
addlib libkrb5support.a
addlib libkrb5.a
addlib libcom_err.a
save
end
EOF

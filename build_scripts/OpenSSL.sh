#!/bin/bash

mkdir build

map_dir=/root/openssl
cp ~/jenkins_util.sh .

cat > build.sh << EOF
#!/bin/bash
cd $map_dir
source jenkins_util.sh
select_toolchain $Compiler
cd build
# aws and mongodb does NOT support no-deprecated
../Configure $cross --prefix=$ARTIFACT_DIR/$TAG --libdir=lib64 \
	no-shared no-apps no-docs no-tests no-legacy no-engine
make -j$(nproc)
make install
rm -rf *
chown -R $(id -u):$(id -g) $ARTIFACT_DIR/$TAG
EOF

chmod +x build.sh
docker run --rm -v $(pwd):$map_dir -v $HOME:$HOME build_env:OpenSSL $map_dir/build.sh

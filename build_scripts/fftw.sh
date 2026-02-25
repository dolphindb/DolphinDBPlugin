TAG=fftw-3.3.10
cp /home/api/src/$TAG.tar.gz .
rm -rf $TAG && tar -xf $TAG.tar.gz && cd $TAG
if [[ -n $CROSS_HOST ]];then
	simd="--enable-neon --enable-armv8-cntvct-el0"
else
	simd="--enable-sse2 --enable-avx --enable-avx2 --enable-avx-128-fma"
    if [[ $(gcc -dumpversion | cut -d. -f1) -ge 8 ]];then
    	simd="$simd --enable-avx512"
    fi
fi
mkdir build && cd build
set_gnu_env
../configure $CROSS_HOST --prefix=$ARTIFACT_DIR/$TAG \
    --enable-static --disable-shared \
    $simd --enable-openmp
make -j$(nproc) -O V=1
make install
# CMakeLists.txt 不支持 NEON
# sed -i 's/cmake_minimum_required (VERSION 3.0)/cmake_minimum_required(VERSION 3.10)/g' CMakeLists.txt
# cmake_all -DENABLE_OPENMP=ON -DBUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF \
#	-DENABLE_AVX=ON -DENABLE_AVX2=ON -DENABLE_SSE=ON -DENABLE_SSE2=ON
ln -s $ARTIFACT_DIR/$TAG $ARTIFACT_DIR/ABI/$TAG

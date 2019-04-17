#! /bin/bash

source script/detect_gcc
source script/discover_os

discover_os

# check if the directory $1/SUITESPARSE exist

if [ -d "$1/SUITESPARSE"  -a -f "$1/SUITESPARSE/include/umfpack.h" ]; then
  echo "SUITESPARSE is already installed"
  exit 0
fi

rm SuiteSparse-5.3.0.tar.gz
wget http://ppmcore.mpi-cbg.de/upload/SuiteSparse-5.3.0.tar.gz
rm -rf SuiteSparse
tar -xf SuiteSparse-5.3.0.tar.gz
if [ $? != 0 ]; then
  echo "Failed to download SuiteSparse"
  exit 1
fi
cd SuiteSparse

if [ x"$CXX" == x"icpc" ]; then
    STS_LIB="-shared-intel -lrt -lifcore"
fi

if [ x"$CXX" == x"g++" ]; then
    OPT_GPP=" -shared -fPIC "
fi


export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$1/OPENBLAS/lib"

if [ x"$platform" == x"cygwin" ]; then
    export PATH="$PATH:$(pwd)/lib"
    echo "$PATH"
fi

echo "Compiling SuiteSparse without CUDA (old variable $CUDA)"
LDLIBS="$STS_LIB -lm" LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$1/OPENBLAS/lib"  make -j $2 "CUDA=no" "BLAS=-L$1/OPENBLAS/lib -lopenblas -pthread" "LAPACK=-lopenblas"
if [ $? != 0 ]; then
  echo "Failed to compile SuiteSparse"
  exit 1
fi
make install "CUDA=no" "INSTALL=$1/SUITESPARSE" "INSTALL_LIB=$1/SUITESPARSE/lib" "INSTALL_INCLUDE=$1/SUITESPARSE/include" "BLAS=-L$1/OPENBLAS/lib -lopenblas -pthread" "LAPACK="
# Mark the installation
echo 1 > $1/SUITESPARSE/version
rm -rf SuiteSparse
rm SuiteSparse-5.3.0.tar.gz

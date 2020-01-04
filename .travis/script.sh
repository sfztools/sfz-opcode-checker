#!/bin/bash

set -ex

mkdir -p build/${INSTALL_DIR} && cd build
/usr/local/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=${PWD}/${INSTALL_DIR} ..

if [[ ${TRAVIS_OS_NAME} == "linux" ]]; then

  make -j$(nproc)
elif [[ ${TRAVIS_OS_NAME} == "osx" ]]; then

  make -j$(sysctl -n hw.ncpu)
fi

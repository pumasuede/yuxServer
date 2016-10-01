#! /bin/bash

cd `dirname $0`
if [ ! -d build ]; then mkdir -p build/{src,test} ; fi

cd build

cd src
cmake ../../src
make && make install
cd ../test
cmake ../../test
make && make install
cd ..

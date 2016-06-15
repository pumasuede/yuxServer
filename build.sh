#! /bin/bash
if [[ ! -d build ]]; then mkdir -p build/{src,test} ; fi

cd build
cd src
cmake ../../src
make
cd ../test
cmake ../../test
make
cd ..

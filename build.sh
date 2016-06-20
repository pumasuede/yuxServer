#! /bin/bash

cd `dirname $0`/build
if [[ ! -d build ]]; then mkdir -p build/{src,test} ; fi

cd src
cmake ../../src
make
cd ../test
cmake ../../test
make
cd ..

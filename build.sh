#!/bin/bash

cd `dirname $0`
type protoc &> /dev/null  || { echo >&2 "protoc does not exist, please install it. Aborting."; exit 1; }
protoc --proto_path=./src/proto --cpp_out ./src/pbout ./src/proto/*.proto

if [ ! -d build ]; then mkdir -p build/{src,test} ; fi

cd build

cd src
cmake ../../src
make && make install
cd ../test
cmake ../../test
make && make install
cd ..

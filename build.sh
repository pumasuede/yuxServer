#!/bin/bash --login

cd `dirname $0`
shopt -s  expand_aliases
shopt expand_aliases
gpb_info=`type protoc`

[ $? -eq 1 ] && { echo >&2 "protoc does not exist, please install it. Aborting."; exit 1; }
protoc --proto_path=./src/proto --cpp_out ./src/pbout ./src/proto/*.proto

gpb_dir=`echo $gpb_info|sed 's/.*[ \`]\//\//g'|sed 's/bin\/protoc.*//g'`
export GPB_DIR=$gpb_dir

if [ ! -d build ]; then mkdir -p build/{src,test} ; fi
cd build

cd src
cmake ../../src
make && make install
cd ../test
cmake ../../test
make && make install
cd ..

#! /bin/bash
cd build
cmake ../src
make
cmake ../test
make
cd ..

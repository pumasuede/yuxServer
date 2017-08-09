project (serverTest)
cmake_minimum_required (VERSION 2.6)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_VERBOSE_MAKEFILE OFF)
SET(CMAKE_CXX_COMPILER "g++")
MESSAGE("Compiler: ${CMAKE_CXX_COMPILER}")

IF (APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
ENDIF()

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

INCLUDE_DIRECTORIES(../src ${GPB_DIR}/include)
LINK_DIRECTORIES(../../install/bin ${GPB_DIR}/lib)

ADD_EXECUTABLE(TestClient TestClient.cpp)
TARGET_LINK_LIBRARIES(TestClient Pbout Base StateMachine protobuf)

ADD_EXECUTABLE(Test Test.cpp)
TARGET_LINK_LIBRARIES(Test Base)

SET(CMAKE_INSTALL_PREFIX ../../install)
INSTALL(TARGETS TestClient Test DESTINATION bin)

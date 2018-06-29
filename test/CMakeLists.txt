project (serverTest)
cmake_minimum_required (VERSION 2.6)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_VERBOSE_MAKEFILE OFF)

SET(CMAKE_CXX_COMPILER "$ENV{CXX_PATH}")
MESSAGE("Compiler: ${CMAKE_CXX_COMPILER}")

IF (APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
ENDIF()

# C++11
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

MESSAGE("BASE DIR : $ENV{BASE_DIR}")
MESSAGE("GPB DIR : $ENV{GPB_DIR}")
MESSAGE("CXX_FLAGS : ${CMAKE_CXX_FLAGS}")

INCLUDE_DIRECTORIES(../src $ENV{GPB_DIR}/include)
LINK_DIRECTORIES(/usr/local/lib $ENV{BASE_DIR}/install/bin $ENV{GPB_DIR}/lib)

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

ADD_EXECUTABLE(TestThread main.cpp)
TARGET_LINK_LIBRARIES(TestThread Base pthread)

ADD_EXECUTABLE(TestClient TestClient.cpp)
TARGET_LINK_LIBRARIES(TestClient Pbout Base StateMachine protobuf)

ADD_EXECUTABLE(TestServer TestServer.cpp)
TARGET_LINK_LIBRARIES(TestServer Base)

ADD_EXECUTABLE(HttpClient TestHttpClient.cpp)
TARGET_LINK_LIBRARIES(HttpClient Base Common)

SET(CMAKE_INSTALL_PREFIX ../../install)
INSTALL(TARGETS HttpClient TestClient TestServer TestThread DESTINATION bin)

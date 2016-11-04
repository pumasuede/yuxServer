project (yuxServer)
cmake_minimum_required (VERSION 2.6)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_VERBOSE_MAKEFILE OFF)

SET(CMAKE_CXX_COMPILER "g++")
MESSAGE("Compiler: ${CMAKE_CXX_COMPILER}")

IF (APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
ENDIF()

MESSAGE("GPB DIR : $ENV{GPB_DIR}")
INCLUDE_DIRECTORIES(. /usr/local/include $ENV{GPB_DIR}/include)
LINK_DIRECTORIES(/usr/local/lib ../../install/bin $ENV{GPB_DIR}/lib)

ADD_SUBDIRECTORY(pbout)
ADD_SUBDIRECTORY(base)
ADD_SUBDIRECTORY(parser)
ADD_SUBDIRECTORY(statemachine)
ADD_SUBDIRECTORY(common)


ADD_EXECUTABLE(yuxServer main.cpp)
TARGET_LINK_LIBRARIES(yuxServer Pbout Common StateMachine)

SET(CMAKE_INSTALL_PREFIX ../../install)
INSTALL(TARGETS yuxServer DESTINATION bin)

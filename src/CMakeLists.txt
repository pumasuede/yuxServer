project (yuxServer)
cmake_minimum_required (VERSION 2.6)

ADD_SUBDIRECTORY(pbout)
ADD_SUBDIRECTORY(base)
ADD_SUBDIRECTORY(statemachine)
ADD_SUBDIRECTORY(../test ../build)

INCLUDE_DIRECTORIES(.)
LINK_DIRECTORIES(/usr/local/lib)

ADD_EXECUTABLE(yuxServer main.cc)
TARGET_LINK_LIBRARIES(yuxServer Pbout Base StateMachine)

SET(CMAKE_INSTALL_PREFIX /workspace/git/yuxServer/install)
INSTALL(TARGETS yuxServer DESTINATION bin)

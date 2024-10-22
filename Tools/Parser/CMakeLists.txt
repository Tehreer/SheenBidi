cmake_minimum_required(VERSION  3.10)
project(SheenBidi)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Headers)
file(GLOB SRC_FILES *.cpp)

option(BUILD_SHARED_LIBS "Build shared library" ON)

if(BUILD_SHARED_LIBS)
set(BUILDSHARE SHARED)
else()
set(BUILDSHARE )
endif()

add_library(parser ${BUILDSHARE} ${SRC_FILES})
cmake_minimum_required(VERSION  3.10)
project(SheenBidi)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Headers)
file(GLOB SRC_FILES Source/*.c)

option(BUILD_SHARED_LIBS "Build shared library" OFF)

if(BUILD_SHARED_LIBS)
set(BUILDSHARE SHARED)
else()
set(BUILDSHARE )
endif()

add_library(sheenbidi ${BUILDSHARE} ${SRC_FILES})

add_subdirectory(Tools/Parser)
add_subdirectory(Tools/Tester)
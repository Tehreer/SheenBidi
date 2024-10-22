cmake_minimum_required(VERSION  3.10)
project(SheenBidi)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../  ${CMAKE_CURRENT_SOURCE_DIR}/../../)

file(GLOB SRC_FILES *.cpp)
file(GLOB UTIL_FILES Utilities/*.cpp)

option(BUILD_SHARED_LIBS "Build shared library" ON)

if(BUILD_SHARED_LIBS)
set(BUILDSHARE SHARED)
else()
set(BUILDSHARE )
endif()

add_executable(tester ${SRC_FILES} ${UTIL_FILES})
target_link_libraries(tester parser sheenbidi)
cmake_minimum_required(VERSION 2.8)
project(ProphetService CXX C)

include_directories("Libs/_FLENS")
include_directories("Libs/cassandra/include")

add_library(libopenblas SHARED IMPORTED)
set_target_properties(libopenblas PROPERTIES IMPORTED_LOCATION Libs/linux/openblas/libopenblas.so)

add_library(libcassandra SHARED IMPORTED)
set_target_properties(libcassandra PROPERTIES IMPORTED_LOCATION Libs/linux/cassandra/libcassandra.so)

add_library(libuv SHARED IMPORTED)
set_target_properties(libuv PROPERTIES IMPORTED_LOCATION Libs/linux/libuv/libuv.so)


FIND_PACKAGE (Threads REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -Wall -DWITH_OPENBLAS")

file(GLOB SOURCE_FILES
	"Prophet.cpp"
	"TaskRunner/*.h"
	"TaskRunner/*.hpp"
	"TaskRunner/*.cpp"
	"ML/*.h"
	"ML/*.hpp"
	"ML/*.cpp"
	"Data/*.h"
	"Data/*.hpp"
	"Data/*.cpp"
	"FileReaders/*.h"
	"FileReaders/*.hpp"
	"FileReaders/*.cpp"
	"Tasks/*.h"
	"Tasks/*.hpp"
	"Tasks/*.cpp")

add_executable(ProphetService ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(ProphetService libopenblas)
TARGET_LINK_LIBRARIES(ProphetService libcassandra)
TARGET_LINK_LIBRARIES(ProphetService libuv)
TARGET_LINK_LIBRARIES(ProphetService ${CMAKE_THREAD_LIBS_INIT})

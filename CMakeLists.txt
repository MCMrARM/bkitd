cmake_minimum_required(VERSION 3.14)
project(bkit C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(Libevent REQUIRED COMPONENTS libevent)
find_package(Libplist REQUIRED)

add_subdirectory(libbridgexpc)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

include_directories(libplistpp2/include)
add_executable(bkitd main.cpp BkConnection.cpp PlistHelper.h Protocol.h)
target_include_directories(bkitd PUBLIC ${LIBEVENT_INCLUDE_DIRS} ${LIBPLIST_INCLUDE_DIRS})
target_link_libraries(bkitd PUBLIC ${LIBEVENT_LIBRARIES} ${LIBPLIST_LIBRARIES} bridgexpc bridgexpc-libevent)
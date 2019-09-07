cmake_minimum_required(VERSION 3.14)
project(bkit C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(Libevent REQUIRED COMPONENTS libevent)
find_package(Libplist REQUIRED)
find_package(LibplistPP REQUIRED)

add_subdirectory(libbridgexpc)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

include_directories(bridge/include)
add_executable(bkitd main.cpp BkConnection.cpp)
target_include_directories(bkitd PUBLIC ${LIBEVENT_INCLUDE_DIRS} ${LIBPLIST_INCLUDE_DIRS} ${LIBPLISTPP_INCLUDE_DIRS})
target_link_libraries(bkitd PUBLIC ${LIBEVENT_LIBRARIES} ${LIBPLIST_LIBRARIES} ${LIBPLISTPP_LIBRARIES} bridgexpc bridgexpc-libevent)
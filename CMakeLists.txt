set(CMAKE_CXX_FLAGS "-std=c++0x -Wall  -gdwarf-3 -g")

cmake_minimum_required(VERSION 3.0)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake-modules/ ${CMAKE_MODULE_PATH}  )

add_library(exception STATIC exception.cpp)
add_library(logger STATIC logger.cpp)
add_library(daemon STATIC daemon.cpp)
add_library(server STATIC server.cpp)
add_library(execute STATIC execute.cpp)
add_executable(daemon_ex daemon.cpp)
target_link_libraries(daemon_ex exception logger server execute)

add_library(client STATIC client.cpp)
add_executable(file-integrity-client client.cpp)
target_link_libraries(file-integrity-client daemon logger exception)

find_package(OpenSSL REQUIRED)
include_directories(${OpenSSL_INCLUDE_DIRS})

find_package(BerkeleyDB REQUIRED)
include_directories(${DB_INCLUDE_DIRS})


add_library(db STATIC BerkeleyDB.cpp)
add_library(file_integrity STATIC file-integrity.cpp)
add_executable(fi-server fi-server.cpp)

target_link_libraries(fi-server daemon logger exception file_integrity execute db  ${BDB_LIBRARY} ${OpenSSL_LIBRARIES})


set(CMAKE_CXX_FLAGS "-std=c++0x -Wall ")

cmake_minimum_required(VERSION 3.0)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} cmake-modules )

add_library(lib_exception STATIC exception.cpp)
add_library(lib_logger STATIC logger.cpp)
add_library(lib_daemon STATIC daemon.cpp)
add_library(lib_server STATIC server.cpp)
add_executable(daemon daemon.cpp)
target_link_libraries(daemon lib_exception lib_logger lib_server)

add_library(lib_client STATIC client.cpp)
add_executable(fi-client client.cpp)
target_link_libraries(fi-client lib_daemon lib_logger lib_exception)


find_package(Boost COMPONENTS regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(MySQL REQUIRED)
include_directories(${MySQL_INCLUDE_DIRS})


find_package(OpenSSL REQUIRED)
include_directories(${OpenSSL_INCLUDE_DIRS})

add_library(lib_mysql STATIC mysql.cpp)
add_library(lib_file_integrity STATIC file-integrity.cpp)
add_executable(fi-server fi-server.cpp)

target_link_libraries(fi-server lib_daemon lib_logger lib_exception lib_file_integrity lib_mysql mysqlcppconn ${Boost_LIBRARIES} ${MySQL_LIBRARIES} ${OpenSSL_LIBRARIES})

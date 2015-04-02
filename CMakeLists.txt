PROJECT(rtt_introspection)
cmake_minimum_required(VERSION 2.6)

SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/.orogen/config")
INCLUDE(rtt_introspectionBase)

add_subdirectory(src)
# FIND_PACKAGE(KDL)
# FIND_PACKAGE(OCL)


cmake_minimum_required(VERSION 3.12.4)
project(electronic_lock C CXX)

set(LV_BUILD_CONF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lv_port_linux" CACHE PATH "Directory containing lv_conf.h" FORCE)

add_subdirectory(lv_port_linux)

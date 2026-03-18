cmake_minimum_required(VERSION 3.28)
project(Crystal_Log LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Crystal_Log STATIC)

target_sources(Crystal_Log
    PUBLIC
        FILE_SET CXX_MODULES FILES
            src/Crystal_Log.ixx
            src/Internal.ixx
            src/Types.ixx
)

target_include_directories(Crystal_Log 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
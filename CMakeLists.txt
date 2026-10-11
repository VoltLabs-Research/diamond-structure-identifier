cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(identify-diamond VERSION 2.0.0 LANGUAGES CXX)

if(NOT TARGET coretoolkit::coretoolkit)
    find_package(coretoolkit REQUIRED)
endif()
if(NOT TARGET common-neighbor-analysis::common-neighbor-analysis)
    find_package(common-neighbor-analysis REQUIRED)
endif()
if(NOT TARGET structure-identification::structure-identification)
    find_package(structure-identification REQUIRED)
endif()

volt_add_plugin(identify-diamond
    MAIN_SOURCE src/plugin.cpp
    DEPENDENCIES
        common-neighbor-analysis::common-neighbor-analysis
        structure-identification::structure-identification
)

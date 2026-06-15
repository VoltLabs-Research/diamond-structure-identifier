cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(identify-diamond VERSION 2.0.0 LANGUAGES CXX)

find_package(coretoolkit REQUIRED)
find_package(common-neighbor-analysis REQUIRED)
find_package(structure-identification REQUIRED)

volt_add_plugin(identify-diamond
    MAIN_SOURCE src/plugin.cpp
    DEPENDENCIES
        common-neighbor-analysis::common-neighbor-analysis
        structure-identification::structure-identification
)

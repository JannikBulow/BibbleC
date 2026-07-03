cmake_minimum_required(VERSION 3.26)

include(FetchContent)
include(GenerateExportHeader)

set(SOURCES
    src/compiler.cpp
)

set(HEADERS
    include/BibbleC/compiler.h
    include/BibbleC/module.h
)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(BibbleC-compiler-shared SHARED ${SOURCES} ${HEADERS})
add_library(BibbleC-compiler-static STATIC ${SOURCES} ${HEADERS})

generate_export_header(BibbleC-compiler-shared
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/BibbleC/api2.h
    EXPORT_MACRO_NAME BIBBLE_EXPORT
    NO_EXPORT_MACRO_NAME BIBBLE_NO_EXPORT
    STATIC_DEFINE BIBBLE_STATIC_DEFINE
)

target_compile_definitions(BibbleC-compiler-static PUBLIC BIBBLE_STATIC_DEFINE)

target_include_directories(BibbleC-compiler-shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_include_directories(BibbleC-compiler-static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(BibbleC-compiler-shared PUBLIC cxx_std_20)
target_compile_features(BibbleC-compiler-static PUBLIC cxx_std_20)

target_link_libraries(BibbleC-compiler-shared BibbleC-framework-shared)
target_link_libraries(BibbleC-compiler-static BibbleC-framework-static)

set_target_properties(BibbleC-compiler-shared PROPERTIES OUTPUT_NAME "bibblec")
set_target_properties(BibbleC-compiler-static PROPERTIES OUTPUT_NAME "bibblec-aio")
cmake_minimum_required(VERSION 3.26)

include(FetchContent)
include(GenerateExportHeader)

FetchContent_Declare(BibblIR
    GIT_REPOSITORY https://github.com/JannikBulow/BibblIR
    GIT_TAG master
)
FetchContent_MakeAvailable(BibblIR)

FetchContent_Declare(vlex
    GIT_REPOSITORY https://github.com/viper-org/vlex
    GIT_TAG master
)
FetchContent_MakeAvailable(vlex)

set(SOURCES
    src/diagnostic/diagnostics.cpp

    src/parser/ast/expression/binary_expression.cpp

    src/parser/ast/node.cpp

    src/parser/parser.cpp

    src/type/integer_type.cpp
)

set(HEADERS
    include/BibbleC/debug/source_pair.h

    include/BibbleC/diagnostic/diagnostics.h

    include/BibbleC/parser/ast/expression/binary_expression.h

    include/BibbleC/parser/ast/node.h

    include/BibbleC/parser/parser.h

    include/BibbleC/scope/scope.h

    include/BibbleC/type/integer_type.h
    include/BibbleC/type/type.h

    include/BibbleC/util/better_strings.h
)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(BibbleC-framework-shared SHARED ${SOURCES} ${HEADERS})
add_library(BibbleC-framework-static STATIC ${SOURCES} ${HEADERS})

generate_export_header(BibbleC-framework-shared
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/BibbleC/api.h
    EXPORT_MACRO_NAME BIBBLEC_EXPORT
    NO_EXPORT_MACRO_NAME BIBBLEC_NO_EXPORT
    STATIC_DEFINE BIBBLEC_STATIC_DEFINE
)

target_compile_definitions(BibbleC-framework-static PUBLIC BIBBLEC_STATIC_DEFINE)

target_include_directories(BibbleC-framework-shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_include_directories(BibbleC-framework-static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(BibbleC-framework-shared PUBLIC cxx_std_20)
target_compile_features(BibbleC-framework-static PUBLIC cxx_std_20)

target_link_libraries(BibbleC-framework-shared BibblIR-framework-shared)
target_link_libraries(BibbleC-framework-static BibblIR-framework-static)

set_target_properties(BibbleC-framework-shared PROPERTIES OUTPUT_NAME "bibblec-backend")
set_target_properties(BibbleC-framework-static PROPERTIES OUTPUT_NAME "bibblec-backend-aio")

#TODO: vlex export api support
vlex_generate(BibbleC-framework-shared src/lexer/lexer.vlex src/lexer include/BibbleC/lexer)
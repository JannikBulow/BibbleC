cmake_minimum_required(VERSION 3.26)

set(SOURCES
    src/main.cpp
)

set(HEADERS
)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_executable(BibbleC-main ${SOURCES} ${HEADERS})

target_include_directories(BibbleC-main PUBLIC include)

target_compile_features(BibbleC-main PUBLIC cxx_std_20)

set_target_properties(BibbleC-main PROPERTIES OUTPUT_NAME "bibblec")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_custom_command(TARGET BibbleC-main PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BibbleC-compiler-shared>
        $<TARGET_FILE_DIR:BibbleC-main>
    )

    add_custom_command(TARGET BibbleC-main PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BibbleC-framework-shared>
        $<TARGET_FILE_DIR:BibbleC-main>
    )

    add_custom_command(TARGET BibbleC-main PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BibblIR-framework-shared>
        $<TARGET_FILE_DIR:BibbleC-main>
    )

    add_custom_command(TARGET BibbleC-main PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BibbleASM-framework-shared>
        $<TARGET_FILE_DIR:BibbleC-main>
    )

    add_custom_command(TARGET BibbleC-main PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BibbleBytecode-shared>
        $<TARGET_FILE_DIR:BibbleC-main>
    )

    if (MSVC)
        add_custom_command(TARGET BibbleC-main PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_PDB_FILE:BibbleC-compiler-shared>
            $<TARGET_PDB_FILE_DIR:BibbleC-main>
        )

        add_custom_command(TARGET BibbleC-main PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_PDB_FILE:BibbleC-framework-shared>
            $<TARGET_PDB_FILE_DIR:BibbleC-main>
        )

        add_custom_command(TARGET BibbleC-main PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_PDB_FILE:BibblIR-framework-shared>
            $<TARGET_PDB_FILE_DIR:BibbleC-main>
        )

        add_custom_command(TARGET BibbleC-main PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_PDB_FILE:BibbleASM-framework-shared>
            $<TARGET_PDB_FILE_DIR:BibbleC-main>
        )

        add_custom_command(TARGET BibbleC-main PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_PDB_FILE:BibbleBytecode-shared>
            $<TARGET_PDB_FILE_DIR:BibbleC-main>
        )
    endif()
endif()
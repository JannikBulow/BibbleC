cmake_minimum_required(VERSION 3.26)
project(BibbleC VERSION 0.1.0 LANGUAGES CXX)

include(CMakePackageConfigHelpers)

add_subdirectory(compiler)
add_subdirectory(framework)
add_subdirectory(main)

configure_package_config_file(
    cmake/BibbleCConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/BibbleCConfig.cmake
    INSTALL_DESTINATION lib/cmake/BibbleC
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/BibbleCConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/BibbleCConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/BibbleCConfigVersion.cmake
    DESTINATION lib/cmake/BibbleC
)

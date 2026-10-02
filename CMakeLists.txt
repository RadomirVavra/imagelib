cmake_minimum_required(VERSION 3.20)

project(imagelib VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(imagelib INTERFACE)
add_library(imagelib::imagelib ALIAS imagelib)
target_compile_features(imagelib INTERFACE cxx_std_20)
target_include_directories(imagelib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

option(IMAGELIB_BUILD_TESTING "Build imagelib tests" ON)
if(IMAGELIB_BUILD_TESTING)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS imagelib EXPORT imagelibTargets)
install(EXPORT imagelibTargets
    FILE imagelibTargets.cmake
    NAMESPACE imagelib::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imagelib)
configure_package_config_file(cmake/imagelibConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/imagelibConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imagelib)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/imagelibConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/imagelibConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/imagelibConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imagelib)
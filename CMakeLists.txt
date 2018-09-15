cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# https://cmake.org/cmake/help/latest/variable/CMAKE_MACOSX_RPATH.html
set(CMAKE_MACOSX_RPATH 1)

# https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html
include(CMakePackageConfigHelpers)

project(FutureConstrictor VERSION 0.1.0 LANGUAGES CXX)
# find_package(stlab REQUIRED CONFIG)
add_library(${PROJECT_NAME}
    private/BoundedConstrictor.cpp
    private/IFutureConstrictor.cpp
    private/SingleConstrictor.cpp
    private/UnboundedConstrictor.cpp
    public/rwols/BoundedConstrictor.hpp
    public/rwols/IFutureConstrictor.hpp
    public/rwols/SingleConstrictor.hpp
    public/rwols/UnboundedConstrictor.hpp
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
# target_link_libraries(${PROJECT_NAME} PUBLIC stlab::stlab)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

# BUG: For some reason, Threads::Threads is not linked transitively.
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

enable_testing()
add_subdirectory(test)

# Enable find_package(FutureConstrictor) from other projects.
set(config_install_dir ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME})
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(namespace rwols::)
set(TARGETS_EXPORT_NAME ${PROJECT_NAME}Targets)
write_basic_package_version_file("${version_config}"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
configure_package_config_file(Config.cmake.in
                              "${project_config}"
                              INSTALL_DESTINATION "${config_install_dir}")

# Install targets, include dirs, and cmake package files.
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY public/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN *.hpp)
install(FILES "${project_config}" "${version_config}"
        DESTINATION "${config_install_dir}")
install(EXPORT ${PROJECT_NAME}Targets NAMESPACE ${namespace}
        DESTINATION "${config_install_dir}")

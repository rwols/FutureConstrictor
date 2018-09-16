cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# https://cmake.org/cmake/help/latest/variable/CMAKE_MACOSX_RPATH.html
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html
include(CMakePackageConfigHelpers)

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)

project(FutureConstrictor VERSION 0.1.0 LANGUAGES CXX)

add_library(ObjectFiles OBJECT
    private/BoundedConstrictor.cpp
    private/IFutureConstrictor.cpp
    private/SingleConstrictor.cpp
    private/UnboundedConstrictor.cpp
    public/rwols/BoundedConstrictor.hpp
    public/rwols/IFutureConstrictor.hpp
    public/rwols/SingleConstrictor.hpp
    public/rwols/UnboundedConstrictor.hpp
)
add_library(${PROJECT_NAME}Static STATIC $<TARGET_OBJECTS:ObjectFiles>)
add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:ObjectFiles>)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}Static)
set_target_properties(ObjectFiles
    PROPERTIES
        INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/public;${CMAKE_CURRENT_BINARY_DIR}"
)
target_include_directories(${PROJECT_NAME}Static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(${PROJECT_NAME}Static PUBLIC cxx_std_14)
target_compile_features(ObjectFiles PUBLIC cxx_std_14)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}Static PUBLIC Threads::Threads)
enable_testing()
add_subdirectory(test)

generate_export_header(
    ${PROJECT_NAME}
    BASE_NAME FC
    EXPORT_MACRO_NAME FC_API
    EXPORT_FILE_NAME rwols/API.hpp
)

# Enable find_package(FutureConstrictor) from other projects.
set(config_install_dir ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME})
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(NAMESPACE rwols::)
set(TARGETS_EXPORT_NAME ${PROJECT_NAME}Targets)
write_basic_package_version_file("${version_config}"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
configure_package_config_file(Config.cmake.in
                              "${project_config}"
                              INSTALL_DESTINATION "${config_install_dir}")

# Install targets, include dirs, and cmake package files.
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}Static
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY public/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN *.hpp)
install(FILES "${project_config}" "${version_config}"
        DESTINATION "${config_install_dir}")
install(EXPORT ${PROJECT_NAME}Targets NAMESPACE ${NAMESPACE}
        DESTINATION "${config_install_dir}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/rwols/API.hpp"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rwols)

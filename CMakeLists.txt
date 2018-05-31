# required for PkgConfig module's IMPORTED_TARGET
cmake_minimum_required(VERSION 3.6)

project(gs-plugin-appimage)

# se this to ON while calling CMake to use system dependency
set(USE_SYSTEM_GNOME_SOFTWARE OFF CACHE BOOL "")

# include external dependencies
add_subdirectory(lib)

# search for dependencies on system
find_package(PkgConfig)

pkg_check_modules(gnome-software REQUIRED IMPORTED_TARGET gnome-software)

set(plugin_deps PkgConfig::gnome-software)

add_library(gs-plugin-appimage SHARED gs-plugin-appimage.c)
target_link_libraries(gs-plugin-appimage libappimage_static libappimageupdate_static ${plugin_deps})
target_compile_definitions(gs-plugin-appimage PRIVATE -DI_KNOW_THE_GNOME_SOFTWARE_API_IS_SUBJECT_TO_CHANGE)

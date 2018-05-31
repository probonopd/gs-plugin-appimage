cmake_minimum_required(VERSION 3.6)

add_subdirectory(AppImageKit EXCLUDE_FROM_ALL)

set(BUILD_QT_UI OFF CACHE BOOL "" FORCE)
add_subdirectory(AppImageUpdate EXCLUDE_FROM_ALL)

#if(NOT USE_SYSTEM_GNOME_SOFTWARE)
#    include(ExternalProject)
#
#    set(install_prefix ${CMAKE_CURRENT_BINARY_DIR}/prefix)
#    file(MAKE_DIRECTORY ${install_prefix})
#
#    set(ENV{PKG_CONFIG_PATH} ${install_prefix}/lib/pkgconfig:${install_prefix}/share/pkgconfig)
#
#    ExternalProject_Add(gnome-software-EXTERNAL
#        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gnome-software
#        CONFIGURE_COMMAND meson --prefix=${install_prefix} <SOURCE_DIR>
#        BUILD_COMMAND ninja all
#        INSTALL_COMMAND "ninja install"
#        EXCLUDE_FROM_ALL 1
#    )
#endif()

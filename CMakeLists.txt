cmake_minimum_required(VERSION 3.10)
project(gst_cameramm)

set(CMAKE_CXX_STANDARD 14)

if (${APPLE})
    # PKG_CONFIG_PATH=/usr/local/opt/libffi/lib/pkgconfig
    set(ENV{PKG_CONFIG_PATH} "/usr/local/opt/libffi/lib/pkgconfig")
endif ()

find_package(PkgConfig REQUIRED)

# region fmt
find_package(fmt REQUIRED)

include_directories(${fmt_INCLUDE_DIRS})

link_directories(${fmt_LIBRARY_DIRS})
# endregion

# region boost
#find_package(Boost REQUIRED COMPONENTS filesystem)
#
#include_directories(${BOOST_INCLUDE_DIRS})
#
#link_directories(${BOOST_LIBRARY_DIRS})
# endregion

# region glib
pkg_check_modules(GLIB REQUIRED glib-2.0)

include_directories(
        ${GLIB_INCLUDE_DIRS}
)

link_directories(
        ${GLIB_LIBRARY_DIRS}
)
# endregion

# region glibmm
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)

include_directories(
        ${GLIBMM_INCLUDE_DIRS}
)

link_directories(
        ${GLIBMM_LIBRARY_DIRS}
)
# endregion

# region gstreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER-BASE REQUIRED gstreamer-base-1.0)
pkg_check_modules(GSTREAMER-APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMER-VIDEO REQUIRED gstreamer-video-1.0)

include_directories(
        ${GSTREAMER_INCLUDE_DIRS}
        ${GSTREAMER-BASE_INCLUDE_DIRS}
        ${GSTREAMER-APP_INCLUDE_DIRS}
        ${GSTREAMER-VIDEO_INCLUDE_DIRS}
)

link_directories(
        ${GSTREAMER_LIBRARY_DIRS}
        ${GSTREAMER-BASE_LIBRARY_DIRS}
        ${GSTREAMER-APP_LIBRARY_DIRS}
        ${GSTREAMER-VIDEO_LIBRARY_DIRS}
)
# endregion

# region gstreamermm
pkg_check_modules(GSTREAMERMM REQUIRED gstreamermm-1.0)

include_directories(
        ${GSTREAMERMM_INCLUDE_DIRS}
)

link_directories(
        ${GSTREAMERMM_LIBRARY_DIRS}
)
# endregion

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(
        gst_camera_SOURCES
        main.cpp
)

set(
        gst_camera_LIBRARIES

        fmt::fmt

#        Boost::filesystem

        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER-BASE_LIBRARIES}
        ${GSTREAMER-APP_LIBRARIES}
        ${GSTREAMER-VIDEO_LIBRARIES}

        ${GSTREAMERMM_LIBRARIES}

        pthread
        ${GLIB_LIBRARIES}
        ${GLIB_GIO_LIBRARIES}
        ${GLIB_GOBJECT_LIBRARIES}

        ${GLIBMM_LIBRARIES}
        ${GLIBMM_GIO_LIBRARIES}
        ${GLIBMM_GOBJECT_LIBRARIES}
)

add_executable(gst_cameramm ${gst_camera_SOURCES})
target_link_libraries(gst_cameramm ${gst_camera_LIBRARIES})
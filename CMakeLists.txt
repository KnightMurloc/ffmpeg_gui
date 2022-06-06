cmake_minimum_required(VERSION 3.16)
project(ffmpeg_gui)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(json)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(${GTKMM_LIBRARY_DIRS})

include_directories(${GTKMM_INCLUDE_DIRS})
#message(${GTKMM_INCLUDE_DIRS})

add_executable(ffmpeg_gui main.cpp lib.cpp lib.h Entry.cpp Entry.h)
target_link_libraries(ffmpeg_gui ${GTKMM_LIBRARIES}  )
target_include_directories(ffmpeg_gui PRIVATE json/include)
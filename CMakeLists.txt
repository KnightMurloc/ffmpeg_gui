cmake_minimum_required(VERSION 3.16)
project(ffmpeg_gui)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra")

add_subdirectory(json)
add_subdirectory(gpu_detect)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(${GTKMM_LIBRARY_DIRS})

include_directories(${GTKMM_INCLUDE_DIRS})
#message(${GTKMM_INCLUDE_DIRS})

add_executable(ffmpeg_gui main.cpp lib.cpp lib.h Entry.cpp Entry.h Form.cpp Form.h)
target_link_libraries(ffmpeg_gui ${GTKMM_LIBRARIES} gpu_detect)
target_include_directories(ffmpeg_gui PRIVATE json/include)
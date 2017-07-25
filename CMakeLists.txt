cmake_minimum_required(VERSION 3.6)
project(Mandlebrot)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Modules")

find_package(Boost 1.64.0 COMPONENTS system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(GMP REQUIRED)

set(SOURCE_FILES main.cpp Bitmap.h Bitmap.cpp BitmapFileHeader.h BitmapInfoHeader.h)
add_executable(Mandlebrot ${SOURCE_FILES})
target_link_libraries(Mandlebrot ${Boost_LIBRARIES})
target_link_libraries(Mandlebrot gmp libgmp.so)
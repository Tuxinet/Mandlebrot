cmake_minimum_required(VERSION 3.6)
project(Mandlebrot)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -fPIC")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Modules")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

find_package(Boost 1.64.0 COMPONENTS system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(GMP REQUIRED)

set(SOURCE_FILES main.cpp Bitmap.h Bitmap.cpp BitmapFileHeader.h BitmapInfoHeader.h)
add_executable(Mandlebrot ${SOURCE_FILES})
target_link_libraries(Mandlebrot ${Boost_LIBRARIES})
target_link_libraries(Mandlebrot mpfr gmp)
cmake_minimum_required(VERSION 3.15)
project(jagent)

#set(BUILD_USE_64BITS on)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)

include_directories(
        # JNI Support
        "C:/Program Files/Java/jdk1.8.0_181/include/win32"
        "C:/Program Files/Java/jdk1.8.0_181/include"
)
find_package(JNI REQUIRED)
message("K")
message("${CMAKE_CXX_FLAGS}")
message("V")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -shared -static-libgcc -static-libstdc++ -std=c++11")

message("${CMAKE_CXX_FLAGS}")
message("W")

add_library(jagent SHARED library.cpp library.h load_ins.h load_ins_win32.cpp)
#target_link_libraries(jagent)

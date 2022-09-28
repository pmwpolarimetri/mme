# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.19...3.24)

#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

project(MME
    VERSION 1.0
    DESCRIPTION "MME Project"
    LANGUAGES CXX)


add_subdirectory(apps)
add_subdirectory(external)
add_subdirectory(mme)

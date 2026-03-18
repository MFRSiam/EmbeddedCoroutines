cmake_minimum_required(VERSION 4.1)
project(EmbeddedCoroutines)

set(CMAKE_CXX_STANDARD 26)

add_executable(EmbeddedCoroutines main.cpp
        EmbCoroutines/EmbCoroutines.cpp
        EmbCoroutines/EmbCoroutines.hpp)

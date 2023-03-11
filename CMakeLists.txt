cmake_minimum_required(VERSION 3.5)

project(sfml-mandelbrot LANGUAGES CXX)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sfml-mandelbrot main.cpp)
target_link_libraries(sfml-mandelbrot sfml-graphics sfml-window sfml-system)
target_link_libraries(sfml-mandelbrot Threads::Threads)

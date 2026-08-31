cmake_minimum_required(VERSION 3.16)

project(CPPwiringPi VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra)

find_library(WIRINGPI_LIB wiringPi REQUIRED)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/01_blink/CMakeLists.txt")
	add_subdirectory(01_blink)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/02_button/CMakeLists.txt")
	add_subdirectory(02_button)
endif()


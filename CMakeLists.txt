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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/03_buttonled/CMakeLists.txt")
	add_subdirectory(03_buttonled)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/04_pwmled/CMakeLists.txt")
	add_subdirectory(04_pwmled)
endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/05_servo/CMakeLists.txt")
# 	add_subdirectory(05_servo)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/06_7segmentled/CMakeLists.txt")
# 	add_subdirectory(06_7segmentled)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/07_shiftregister/CMakeLists.txt")
# 	add_subdirectory(07_shiftregister)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/08_i2cadc/CMakeLists.txt")
# 	add_subdirectory(08_i2cadc)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/09_i2cioexpander/CMakeLists.txt")
# 	add_subdirectory(09_i2cioexpander)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/10_accelerometergyro/CMakeLists.txt")
# 	add_subdirectory(10_accelerometergyro)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/11_motordriver/CMakeLists.txt")
# 	add_subdirectory(11_motordriver)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/12_ledmatrix/CMakeLists.txt")
# 	add_subdirectory(12_ledmatrix)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/13_sensehatjoystick/CMakeLists.txt")
# 	add_subdirectory(13_sensehatjoystick)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/14_envsensors/CMakeLists.txt")
# 	add_subdirectory(14_envsensors)
# endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/15_imuorientation/CMakeLists.txt")
	add_subdirectory(15_imuorientation)
endif()
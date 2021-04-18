cmake_minimum_required(VERSION 3.16.3)

project(FPGA_BUILD)
set(PROJECT_VERSION 0.01)

add_executable(${PROJECT_NAME} main.cpp)

# add a sub-level CMakeLists.txt file
add_subdirectory(libethash-fpga)

# include the directory for the '#include<FPGAMiner.h>' line
target_include_directories(${PROJECT_NAME} PUBLIC libethash-fpga)

target_link_directories(${PROJECT_NAME} PRIVATE libethash-fpga)

target_link_libraries(${PROJECT_NAME} ethash-fpga)




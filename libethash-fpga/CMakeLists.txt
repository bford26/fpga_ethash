# A custom command and target to turn the OpenCL kernel into a byte array header
# The normal build depends on it properly and if the kernel file is changed, then
# a rebuild of libethash-cl should be triggered

#TODO: clean up the copy&pasting here
# add_custom_command(
# 	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ethash.h
# 	COMMAND ${CMAKE_COMMAND} ARGS
# 	-DBIN2H_SOURCE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/kernels/cl/ethash.cl"
# 	-DBIN2H_VARIABLE_NAME=ethash_cl
# 	-DBIN2H_HEADER_FILE="${CMAKE_CURRENT_BINARY_DIR}/ethash.h"
# 	-P "${CMAKE_CURRENT_SOURCE_DIR}/bin2h.cmake"
# 	COMMENT "Generating OpenCL Kernel Byte Array"
# 	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kernels/cl/ethash.cl
# )

# add_custom_target(clbin2h_stable DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ethash.h ${CMAKE_CURRENT_SOURCE_DIR}/cl/ethash.cl)

# set(SOURCES
# 	CLMiner.h CLMiner.cpp
# 	${CMAKE_CURRENT_BINARY_DIR}/ethash.h
# )

set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(..)


# =================== CREATE HOST LIB FILE =======================



add_library(ethash-fpga FPGAMiner.cpp FPGAMiner.h xcl2.hpp xcl2.cpp)

# find_package(OpenCL REQUIRED)

target_include_directories(ethash-fpga PUBLIC $ENV{XILINX_XRT}/include )

target_link_directories(ethash-fpga PRIVATE ${XILINX_XRT}/lib)

target_link_libraries(ethash-fpga PUBLIC xrt_coreutil OpenCL pthread rt stdc++)

target_link_libraries(ethash-fpga PUBLIC ethcore ethash::ethash)
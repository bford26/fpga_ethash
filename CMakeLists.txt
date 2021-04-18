cmake_minimum_required(VERSION 3.16.3)

# ==================== INCLUDES ========================== #

include(cmake/cable/bootstrap.cmake)
include(CableBuildInfo)
include(CableBuildType)
include(CableToolchains)
include(HunterGate)

include(defaults/HunterCacheServers)

set(HUNTER_CONFIGURATION_TYPES Release CACHE STRING "Build type of Hunter packages")
set(HUNTER_JOBS_NUMBER 6 CACHE STRING "Number of parallel builds used by Hunter")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.112.tar.gz"
    SHA1 "4b894e1d5d203f0cc9a77431dbb1b486ab6f4430"
    LOCAL
)

cable_set_build_type(DEFAULT RelWithDebInfo CONFIGURATION_TYPES Debug Release RelWithDebInfo)


# ====================== Project Details ======================== #

project(FPGA_BUILD)
set(PROJECT_VERSION 0.01)

set(FPGA_PLATFORM "u280")

## Xilinx
set(XILINX_VITIS $ENV{XILINX_VITIS})
set(XILINX_VIVADO $ENV{XILINX_VIVADO})
set(XCL_KERNEL $ENV{XCL_KERNEL})
set(XCL_OPT_INDEX $ENV{XCL_OPT_INDEX})
set(XCL_EMULATION_MODE $ENV{XCL_EMULATION_MODE})
set(XCL_CU $ENV{XCL_CU})
set(XCL_PLATFORM $ENV{XCL_PLATFORM})
set(XCL_OPTS none buffered dataflow distributed vec h2k k2k)
# list(GET XCL_OPTS ${XCL_OPT_INDEX} XCL_OPT)

## Xilinx Runtime
set(XILINX_XRT $ENV{XILINX_XRT})
set(XRT_PROFILE "true")
set(XRT_TIMELINE_TRACE "true")
set(XRT_DATA_TRANSFER_TRACE "fine")

# ============================================== #




# ====================== BOOST, jsoncpp, ethash ======================== #


hunter_add_package(Boost COMPONENTS system filesystem thread)
find_package(Boost CONFIG REQUIRED COMPONENTS system filesystem thread)

hunter_add_package(jsoncpp)
find_package(jsoncpp CONFIG REQUIRED)

hunter_add_package(ethash)
find_package(ethash CONFIG REQUIRED)


# ============================================== #














add_executable(${PROJECT_NAME} main.cpp)

# add a sub-level CMakeLists.txt file
add_subdirectory(libethash-fpga)

# include the directory for the '#include<FPGAMiner.h>' line
target_include_directories(${PROJECT_NAME} PUBLIC libethash-fpga)

target_link_directories(${PROJECT_NAME} PRIVATE libethash-fpga)

target_link_libraries(${PROJECT_NAME} ethash-fpga)




cmake_minimum_required(VERSION 3.16.3)

# ==================== INCLUDES ========================== #




# ============================================== #




# ====================== BOOST, jsoncpp, ethash ======================== #



include(cmake/cable/bootstrap.cmake)

include(CableBuildInfo)
include(CableBuildType)
include(CableToolchains)
include(HunterGate)

include(defaults/HunterCacheServers)

cable_configure_toolchain(DEFAULT cxx11)

set(HUNTER_CONFIGURATION_TYPES Release CACHE STRING "Build type of Hunter packages")
set(HUNTER_JOBS_NUMBER 6 CACHE STRING "Number of parallel builds used by Hunter")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.112.tar.gz"
    SHA1 "4b894e1d5d203f0cc9a77431dbb1b486ab6f4430"
    LOCAL
)

cable_set_build_type(DEFAULT Release CONFIGURATION_TYPES Release RelWithDebInfo)


project(ethminer)
set(PROJECT_VERSION 0.69.420.0)


# ====================== Project Details ======================== #

# project(FPGA_BUILD)
# set(PROJECT_VERSION 0.01)

# set(FPGA_PLATFORM "u280")

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


# option(ETHASHCL "Build with OpenCL mining" ON)
# option(ETHASHCUDA "Build with CUDA mining" ON)
# option(ETHASHCPU "Build with CPU mining (only for development)" OFF)
# option(ETHDBUS "Build with D-Bus support" OFF)
# option(APICORE "Build with API Server support" ON)
option(BINKERN "Install AMD binary kernels" ON)
option(DEVBUILD "Log developer metrics" ON)
option(USE_SYS_OPENCL "Build with system OpenCL" ON)
option(ETHASHFPGA "Build with FPGA mining" ON)

# propagates CMake configuration options to the compiler
function(configureProject)
    if (BINKERN)
        add_definitions(-DBIN_KERN)
    endif()
    if (DEVBUILD)
        add_definitions(-DDEV_BUILD)
    endif()
    if (USE_SYS_OPENCL)
        add_definitions(-DUSE_SYS_OPENCL)
    endif()
    if(ETHASHFPGA)
        add_definitions(-DETH_ETHASHFPGA)        
    endif()
endfunction()

hunter_add_package(Boost COMPONENTS system filesystem thread)
find_package(Boost CONFIG REQUIRED COMPONENTS system filesystem thread)

hunter_add_package(jsoncpp)
find_package(jsoncpp CONFIG REQUIRED)

hunter_add_package(ethash)
find_package(ethash CONFIG REQUIRED)

configureProject()

if(APPLE)
    set(USE_SYS_OPENCL ON)
endif()

message("----------------------------------------------------------------------------")
message("-- CMake ${CMAKE_VERSION}")
message("-- Build ${CMAKE_BUILD_TYPE} / ${CMAKE_SYSTEM_NAME}")
message("----------------------------------------------------------------- components")
# message("-- ETHASHCL         Build OpenCL components                      ${ETHASHCL}")
# message("-- ETHASHCUDA       Build CUDA components                        ${ETHASHCUDA}")
# message("-- ETHASHCPU        Build CPU components (only for development)  ${ETHASHCPU}")
# message("-- ETHDBUS          Build D-Bus components                       ${ETHDBUS}")
# message("-- APICORE          Build API Server components                  ${APICORE}")
message("-- BINKERN          Install AMD binary kernels                   ${BINKERN}")
message("-- DEVBUILD         Build with dev logging                       ${DEVBUILD}")
message("-- USE_SYS_OPENCL   Build with system OpenCL                     ${USE_SYS_OPENCL}")
message("-- ETHASHFGPA       Build FPGA components                        ${ETHASHFPGA}")
message("----------------------------------------------------------------------------")
message("")

include(EthCompilerSettings)
if(UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif()

cable_add_buildinfo_library(PROJECT_NAME ${PROJECT_NAME})

add_subdirectory(libdevcore)
add_subdirectory(libethcore)
add_subdirectory(libhwmon)
add_subdirectory(libpoolprotocols)

if(ETHASHFPGA)
    add_subdirectory(libethash-fpga)
endif()

add_subdirectory(ethminer)



# if(WIN32)
#     set(CPACK_GENERATOR ZIP)
# else()
#     set(CPACK_GENERATOR TGZ)
# endif()


# set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
# set(CPACK_PACKAGE_CHECKSUM SHA256)
# set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
# include(CPack)


# ============================================== #





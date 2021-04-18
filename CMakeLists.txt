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


# ============================================== #

project(FPGA_BUILD)
set(PROJECT_VERSION 0.01)

# ============================================== #


add_executable(${PROJECT_NAME} main.cpp)

# add a sub-level CMakeLists.txt file
add_subdirectory(libethash-fpga)

# include the directory for the '#include<FPGAMiner.h>' line
target_include_directories(${PROJECT_NAME} PUBLIC libethash-fpga)

target_link_directories(${PROJECT_NAME} PRIVATE libethash-fpga)

target_link_libraries(${PROJECT_NAME} ethash-fpga)




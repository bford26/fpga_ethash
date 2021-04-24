cmake_policy(SET CMP0015 NEW)

set(CMAKE_CXX_STANDARD 14)

aux_source_directory(. SRC_LIST)

include_directories(BEFORE ..)

set(EXECUTABLE ethminer1)

file(GLOB HEADERS "*.h")

add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

hunter_add_package(CLI11)
find_package(CLI11 CONFIG REQUIRED)

target_link_libraries(ethminer1 PRIVATE ethcore poolprotocols devcore ethminer-buildinfo CLI11::CLI11 Boost::system Boost::thread)

if(ETHASHFPGA)
	target_include_directories(ethminer1 PUBLIC $ENV{XILINX_XRT}/include)
	target_link_directories(ethminer1 PUBLIC $ENV{XILINX_XRT}/lib)	
	target_link_libraries(ethminer1 PUBLIC xrt_coreutil OpenCL pthread rt stdc++)
endif(ETHASHFPGA)

include(GNUInstallDirs)
install(TARGETS ethminer1 DESTINATION ${CMAKE_INSTALL_BINDIR})
if(MSVC)
    install(FILES $<TARGET_PDB_FILE:ethminer1> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
endif()
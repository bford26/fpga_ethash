set(CMAKE_CXX_STANDARD 14)

set(SOURCES
	EthashAux.h EthashAux.cpp
	Farm.cpp Farm.h
	Miner.h Miner.cpp
)

include_directories(BEFORE ..)



add_library(ethcore ${SOURCES})
target_link_libraries(ethcore PUBLIC devcore ethash::ethash PRIVATE hwmon)

# if(ETHASHCL)
# 	target_link_libraries(ethcore PRIVATE ethash-cl)
# endif()
# if(ETHASHCUDA)
# 	target_link_libraries(ethcore PUBLIC ethash-cuda)
# endif()

if(ETHASHFPGA)
	target_link_libraries(ethcore PUBLIC ethash-fpga)
	target_include_directories(ethcore PUBLIC $ENV{XILINX_XRT}/include)
endif()

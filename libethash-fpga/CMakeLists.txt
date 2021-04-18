# file(GLOB sources "*.cpp")
# file(GLOB headers "*.h")



# will create the platform folders for deploying app
# add_custom_target(build-time-make-platform-directory ALL COMMAND ${CMAKE_COMMAND} -E make_directory /libethash-fpga/${FPGA_PLATFORM})
# add_custom_target(build-time-make-platform-hw-directory ALL COMMAND ${CMAKE_COMMAND} -E make_directory /libethash-fpga/${FPGA_PLATFORM}/hw)
# add_custom_target(build-time-make-platform-hw_emu-directory ALL COMMAND ${CMAKE_COMMAND} -E make_directory /libethash-fpga/${FPGA_PLATFORM}/hw_emu)
# add_custom_target(build-time-make-platform-sw_emu-directory ALL COMMAND ${CMAKE_COMMAND} -E make_directory /libethash-fpga/${FPGA_PLATFORM}/sw_emu)


# CREATE HOST LIB FILE
add_library(ethash-fpga FPGAMiner.cpp FPGAMiner.h)

target_include_directories(ethash-fpga PUBLIC ${XILINX_XRT}/include)
target_link_directories(ethash-fpga PUBLIC ${XILINX_XRT}/lib)
target_link_libraries(ethash-fpga PUBLIC OpenCL pthread rt stdc++)






# execute_process(COMMAND source /scripts/xilinx_setup.sh)

# CREATE EMU JSON FILE
execute_process(COMMAND emconfigutil --platform xilinx_u280_xdma_201920_3 --nd 1)
# add_custom_target(build-time-emconfigutil ALL COMMAND emconfigutil --platform xilinx_u280_xdma_201920_3 --nd 1)

# CREATE XCLBIN 
# add_custom_target(xilinx_xclbin ALL COMMAND v++ -c -t sw_emu --config ./configs/${FPGA_PLATFORM}.cfg -k kernel kernel.cpp -o ./${FPGA_PLATFORM}/sw_emu/kernel.xo)
# target_include_directories(xilinx_xclbin PUBLIC )







# execute_process(COMMAND g++ -Wall -g -std=c++11 ./src/host.cpp -o app.exe -I${XILINX_XRT}/include/ -L${XILINX_XRT}/lib/ -lOpenCL -lpthread -lrt -lstdc++)
execute_process(COMMAND v++ -c -t sw_emu --config ./src/u280.cfg -k vadd -I./src ./src/vadd.cpp -o ./build/vadd.xo)
execute_process(COMMAND v++ -l -t sw_emu --config ./src/u280.cfg ./build/vadd.xo -o vadd.xclbin)



# add_library(ethash-fpga ${sources} ${headers})

# installs 
# install(TARGETS ethash-fpga DESTINATION lib)
# install(FILES ${headers} DESTINATION include)

# target_link_libraries(ethash-fpga ethcore ethash::ethash Boost::thread)
# target_include_directories(ethash-fgpa PRIVATE .. ${CMAKE_CURRENT_BINARY_DIR})

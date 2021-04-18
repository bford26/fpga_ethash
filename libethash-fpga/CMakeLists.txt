file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(ethash-fpga ${sources} ${headers})

# install(TARGETS ethash-fpga DESTINATION lib)
# install(FILES ${headers} DESTINATION include)

# target_link_libraries(ethash-fpga ethcore ethash::ethash Boost::thread)
# target_include_directories(ethash-fgpa PRIVATE .. ${CMAKE_CURRENT_BINARY_DIR})

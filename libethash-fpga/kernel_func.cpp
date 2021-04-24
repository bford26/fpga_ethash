
#include "experimental/xrt_kernel.h"
#include "experimental/xrt_aie.h"

#define DATA_SIZE 4096


int main()
{
    std::string binaryFile;
    size_t vector_size_bytes sizeof(int) * DATA_SIZE;
    cl_int err;
    cl::Context context;
    cl::Kernel knrl_ethash;
    cl::CommandQueue q;

    // OPENCL HOST CODE AREA START

    // get_xil_devices() is a utility API which will find the xilinx
    // platforms and will return list of devices connected to Xilinx platform
    auto devices = xcl::get_xil_devices();
    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;

    for (unsigned int i = 0; i < devices.size(); i++)
    {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, NULL, NULL, NULL, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context, {device}, bins, NULL, &err);
        if (err != CL_SUCCESS)
        {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        }
        else
        {
            std::cout << "Device[" << i << "]: program successful!\n";
            OCL_CHECK(err, krnl_ethash = cl::Kernel(program, "ethash", &err));
            valid_device = true;
            break; // we break because we found a valid device
        }
    }

    if (!valid_device)
    {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }










    return 0;
}






// unsigned int dev_index = 0;
// auto device = xrt::device(dev_index);
// auto xclbin_uuid = device.load_xclbin("kernel.xclbin");

// // get kernel from xclbin
// auto krnl = xrt::kernel(device, xclbin_uuid, name);



// // transferring data from host to device - example
// auto input_buffer = xrt::bo(device, buffer_size_in_bytes, bank_grp_idx_0);
// // Prepare the input data
// int buff_data[data_size];
// for (auto i=0; i<data_size; ++i)
// {
//     buff_data[i] = i;
// }
// input_buffer.write(buff_data);
// input_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);

















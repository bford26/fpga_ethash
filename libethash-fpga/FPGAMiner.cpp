#include <libethcore/Farm.h>
#include <ethash/ethash.hpp>

#include "FPGAMiner.h"

using namespace dev;
using namespace eth;

#define DATA_SIZE 4096

struct FPGAChannel : public LogChannel
{
    static const char* name() {return EthOrange "fp";}
    static const int verbosity = 2;
    static const bool debug = false;
};
#define fpgalog clog(FPGAChannel)

FPGAMiner::FPGAMiner(unsigned _index, FPGASettings _settings, DeviceDescriptor& _device)
    : Miner("fpga-", _index), m_settings(_settings)
{
    m_deviceDescriptor = _device;
}

FPGAMiner::~FPGAMiner()
{
    DEV_BUILD_LOG_PROGRAMFLOW(fpgalog, "fp-" << m_index << " FPGAMiner::~FPGAMiner() begin");
    stopWorking();
    kick_miner();
    DEV_BUILD_LOG_PROGRAMFLOW(fpgalog, "fp-" << m_index << " CLMiner::~FPGAMiner() end");
}

void FPGAMiner::enumDevices(std::map<string, DeviceDescriptor>& _DevicesCollection)
{   

    std::vector<cl::Platform> platforms;

    cl_int clErr = cl::Platform::get(&platforms);
    if(clErr != CL_SUCCESS)
    {
        printf("Error: Failed to get platforms...\n");
        exit(1);
    }


    unsigned int xilinx_index;
    std::string sTempParam;

    for( auto& plat : platforms )
    {
        clErr = plat.getInfo(CL_PLATFORM_VENDOR, &sTempParam);
    }







    // cl_platform_id xilinx_platform_id;
    // cl_uint platform_count;
    // cl_platform_id* platforms;
    //cl_int clErr = clGetPlatformIDs(16, platforms, &platform_count);
    // if(clErr != CL_SUCCESS)
    // {
    //     printf("Error: Failed to get platform IDs...\n");
    //     exit(1);
    // }


    // find xilinx platform
    // for (unsigned int iplat = 0; iplat < platform_count; iplat++)
    // {
    //     clErr = clGetPlatformInfo(platforms[iplat], CL_PLATFORM_VENDOR, 1000, (void*)cl_platform_vendor, NULL);

    //     if(clErr != CL_SUCCESS)
    //     {
    //         printf("Error: Failed to get platform info...\n");
    //         exit(1);
    //     }

    //     if (strcmp(cl_platform_vendor, "Xilinx") == 0)
    //     {
    //         // xilinx platform found
    //         xilinx_platform_id = platforms[iplat];
    //     }
    // }
    
    
    // cl_uint num_devices;
    // cl_device_id devices[16];
    // char cl_device_name[1001];

    // clErr = clGetDeviceIDs(xilinx_platform_id, CL_DEVICE_TYPE_ACCELERATOR, 16, devices, &num_devices);
    // if(clErr != CL_SUCCESS)
    // {
    //     printf("Error: Failed to get device IDs...\n");
    //     exit(1);
    // }

    // printf("INFO: Found %d devices\n", num_devices);

    // // iterate all devices and select target
    // for (uint i=0; i<num_devices; i++)
    // {
    //     clErr = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 1024, cl_device_name, 0);
    //     if(clErr != CL_SUCCESS)
    //     {
    //         printf("Error: Failed to get device info...\n");
    //         exit(1);
    //     }

    //     printf("CL_DEVICE_NAME %s\n", cl_device_name);





    // }

    
    // // create a context for each device or subdevice
    // auto xil_context = clCreateContext(0,1, &device_id, NULL, NULL, &clErr);
    // if(clErr != CL_SUCCESS)
    // {
    //     printf("Error: Failed to create xil_context...\n");
    //     exit(1);
    // }


    
    
    




    
    




    


}

void FPGAMiner::search(const dev::eth::WorkPackage& w)
{


    // uint32_t current_index;
    // for(current_index=0; current_index<m_settings.streams; current_index++, start_nonce += m_batch_size)
    // {
    // }


}


bool FPGAMiner::initDevice()
{
    
    
    // fpga_device = xrt::device(dev_index);
    // auto xclbin_uuid = fpga_device.load_xclbin("kernel_func.xclbin");


    return true;
}

bool FPGAMiner::initEpoch_internal()
{


    return true;
}

void FPGAMiner::kick_miner()
{

}

void FPGAMiner::workLoop()
{

    uint32_t zero_x3[3] = {0,0,0};
    uint64_t startNonce = 0;

    WorkPackage current;

    current.header = h256();

    if(!initDevice())
        return;
    
    try
    {
        while(0)
        {

        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    


}












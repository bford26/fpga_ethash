#include <libethcore/Farm.h>
#include <ethash/ethash.hpp>

#include "FPGAMiner.h"

using namespace dev;
using namespace eth;

// WARNING: Do not change the value of the following constant
// unless you are prepared to make the neccessary adjustments
// to the assembly code for the binary kernels.
const size_t c_maxSearchResults = 4;

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
    
    cl_int clErr;
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    std::string platformName;
    bool found_device = false;

    cl::Platform xilinx_platform;
    int i;
    for(i=0; i<platforms.size(); i++)
    {
        xilinx_platform = platforms[i];
        platformName = xilinx_platform.getInfo<CL_PLATFORM_NAME>(&clErr);
        if(platformName == "Xilinx")
        {
            std::cout << "INFO: Found Xilinx Platform\n";
            // std::cout << "clPlatformId : " << i << std::endl << std::endl;
            break; 
        }
    }
    if(i == platforms.size())
    {
        std::cout << "ERROR: Failed to find Xilinx platform\n\n";
        exit(EXIT_FAILURE);
    }

    std::string platformVersion = xilinx_platform.getInfo<CL_PLATFORM_VERSION>();
    unsigned int platformVersionMajor = std::stoi(platformVersion.substr(7, 1));
    unsigned int platformVersionMinor = std::stoi(platformVersion.substr(9, 1));



    


    // Get the FPGA devices 
    std::vector<cl::Device> devices;
    devices.clear();
    xilinx_platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);

    cl::Device device;
    if(devices.size())
    { 
        device = devices[0];
        found_device = true;  
    }

    if (found_device == false)
    {
        std::cout << "Error: Unable to find Target Device "
            << device.getInfo<CL_DEVICE_NAME>() << std::endl;
	    exit(EXIT_FAILURE);
	}






    unsigned int dIdx = 0;
    for(auto& device : devices)
    {

        string uniqueId;
        std::ostringstream s;
        s << "Xil:" << i << "." << dIdx;
        uniqueId = s.str();

        DeviceDescriptor deviceDescriptor;
    
        if(_DevicesCollection.find(uniqueId) != _DevicesCollection.end())
            deviceDescriptor = _DevicesCollection[uniqueId];
        else
            deviceDescriptor = DeviceDescriptor();

        deviceDescriptor.clPlatformVersion = platformVersion;
        deviceDescriptor.clPlatformVersionMajor = platformVersionMajor;
        deviceDescriptor.clPlatformVersionMinor = platformVersionMinor;


        deviceDescriptor.clPlatformId = i;
        deviceDescriptor.clPlatformName = platformName;
        deviceDescriptor.clPlatformType = ClPlatformTypeEnum::Xilinx;
        

        // Fill in the blanks by OpenCL means
        deviceDescriptor.name = device.getInfo<CL_DEVICE_NAME>();
        deviceDescriptor.type = DeviceTypeEnum::Accelerator;
        deviceDescriptor.uniqueId = uniqueId;
        deviceDescriptor.fpgaDetected = true;

        deviceDescriptor.clDeviceOrdinal = dIdx;

        deviceDescriptor.clName = deviceDescriptor.name;
        deviceDescriptor.clDeviceVersion = device.getInfo<CL_DEVICE_VERSION>();
        deviceDescriptor.clDeviceVersionMajor =
            std::stoi(deviceDescriptor.clDeviceVersion.substr(7, 1));
        deviceDescriptor.clDeviceVersionMinor =
            std::stoi(deviceDescriptor.clDeviceVersion.substr(9, 1));
        deviceDescriptor.totalMemory = device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
        deviceDescriptor.clMaxMemAlloc = device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
        deviceDescriptor.clMaxWorkGroup = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
        deviceDescriptor.clMaxComputeUnits = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();

        // Apparently some 36 CU devices return a bogus 14!!!
            deviceDescriptor.clMaxComputeUnits =
                deviceDescriptor.clMaxComputeUnits == 14 ? 36 : deviceDescriptor.clMaxComputeUnits;

        // Upsert Devices Collection
        _DevicesCollection[uniqueId] = deviceDescriptor;
        ++dIdx;

    }

    // printf("\nFPGAMiner::enumDevices end\n");

}

bool FPGAMiner::initDevice()
{
    
    // LookUp device
    // Load available platforms
    cl_int clErr;
    std::vector<cl::Platform> platforms;
    
    cl::Platform::get(&platforms);
    if (platforms.empty())
    {
        printf("\nno platforms\n");
        return false;
    }
    
    cl::Platform xilinx_platform;
    int pIdx;
    for(pIdx=0; pIdx <platforms.size(); pIdx++)
    {
        xilinx_platform = platforms[pIdx];
        
        std::string platformName = xilinx_platform.getInfo<CL_PLATFORM_NAME>(&clErr);
        if(platformName == "Xilinx")
        {
            // std::cout << "clPlatformId : " << pIdx << std::endl << std::endl;
            m_deviceDescriptor.clPlatformId = pIdx;
            break;
        }
    }

    // Get the FPGA devices 
    std::vector<cl::Device> devices;
    devices.clear();
    
    // printf("\nDevice Test\n");
    
    xilinx_platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
    
    // printf("\nPassed\n");


    if (devices.empty())
    {
        printf("\nno devices\n");
        return false;
    }
    
    // int dIdx;
    // for(dIdx=0; dIdx<devices.size(); dIdx++)
    // {
    //     std::cout << devices[dIdx].getInfo<CL_DEVICE_NAME>() << "\n";
    // }

    m_device = devices[m_deviceDescriptor.clDeviceOrdinal];

    if(m_deviceDescriptor.clPlatformType == ClPlatformTypeEnum::Xilinx)
    {
        fpgalog << "FPGAMiner::initDevice()\n";
        m_hwmoninfo.deviceType = HwMonitorInfoType::UNKNOWN;
        m_hwmoninfo.devicePciId = m_deviceDescriptor.uniqueId;
        m_hwmoninfo.deviceIndex = -1;
        m_settings.noBinary = true;
    }
    else
    {   
        cout << "Platform : " << (int)m_deviceDescriptor.clPlatformType << "\n";
        fpgalog << "Unrecognized Platform\n";
        return false;
    }

    ostringstream s;
    s << "Using Device : " << m_deviceDescriptor.uniqueId << " " << m_deviceDescriptor.clName;

    if(!m_deviceDescriptor.clNvCompute.empty())
        s << " (Compute " + m_deviceDescriptor.clNvCompute + ")";
    else
        s << " " << m_deviceDescriptor.clDeviceVersion;

    s << " Memory : " << dev::getFormattedMemory((double)m_deviceDescriptor.totalMemory);
    s << " (" << m_deviceDescriptor.totalMemory << " B)";
    fpgalog << s.str();


    // printf("\nInit True\n");
    return true;
}

bool FPGAMiner::initEpoch_internal()
{
    cl_int err;
    auto startInit = std::chrono::steady_clock::now();
    size_t RequiredMemory = (m_epochContext.dagSize);

    // Release the pause flag if any
    resume(MinerPauseEnum::PauseDueToInsufficientMemory);
    resume(MinerPauseEnum::PauseDueToInitEpochError);

    // Check whether the current device has sufficient memory every time we recreate the dag
    if (m_deviceDescriptor.totalMemory < RequiredMemory)
    {
        fpgalog << "Epoch " << m_epochContext.epochNumber << " requires "
              << dev::getFormattedMemory((double)RequiredMemory) << " memory. Only "
              << dev::getFormattedMemory((double)m_deviceDescriptor.totalMemory)
              << " available on device.";
        pause(MinerPauseEnum::PauseDueToInsufficientMemory);
        return true;  // This will prevent to exit the thread and
                      // Eventually resume mining when changing coin or epoch (NiceHash)
    }

    fpgalog << "Generating split DAG + Light (total): "
          << dev::getFormattedMemory((double)RequiredMemory);

    try
    {
        // char options[256] = {0};
        // int computeCapability = 0;


        fpgalog << "INFO: loading xclbin kernel_func.xclbin";
        // printf("\nINFO: loading xclbin kernel_func.xclbin\n\n");

        std::ifstream bin_file("kernel_func.xclbin", std::ifstream::binary);

        if( !bin_file.is_open() )
        {
            fpgalog << "ERROR: failed to open xclbin file";
            return false;
        }

        bin_file.seekg (0, bin_file.end);
        unsigned nb = bin_file.tellg();
        bin_file.seekg (0, bin_file.beg);
        char *buf = new char [nb];
        bin_file.read(buf, nb);

        cl::Program::Binaries bins;
        bins.push_back({buf,nb});

        // create context
        m_context.clear();
        m_context.push_back(cl::Context(m_device, nullptr, nullptr, nullptr, &err));

        m_queue.clear();
        m_queue.push_back(cl::CommandQueue(m_context[0], m_device, CL_QUEUE_PROFILING_ENABLE/*CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE*/, &err));


        m_dagItems = m_epochContext.dagNumItems;

        
        cl::Program program(m_context[0], {m_device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << m_deviceDescriptor.clDeviceIndex << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << m_deviceDescriptor.clDeviceIndex << "]: program successful!\n";
            // break; // we break because we found a valid device
        }

        
        // OCL_CHECK(err, kernel_= cl::Kernel(program, "krnl_vadd", &err));



        // create dag buffer

        try
        {
            fpgalog << "Creating DAG buffer, size: "
                    << dev::getFormattedMemory((double)m_epochContext.dagSize)
                    << ", free: "
                    << dev::getFormattedMemory(
                            (double)(m_deviceDescriptor.totalMemory - RequiredMemory));
            m_dag.clear();

            if(m_epochContext.dagNumItems & 1)
            {
                m_dag.push_back(
                    cl::Buffer(m_context[0], CL_MEM_READ_ONLY, m_epochContext.dagSize / 2 + 64));
                m_dag.push_back(
                    cl::Buffer(m_context[0], CL_MEM_READ_ONLY, m_epochContext.dagSize / 2 - 64));
            }
            else
            {
                m_dag.push_back(
                    cl::Buffer(m_context[0], CL_MEM_READ_ONLY, (m_epochContext.dagSize) / 2));
                m_dag.push_back(
                    cl::Buffer(m_context[0], CL_MEM_READ_ONLY, (m_epochContext.dagSize) / 2));
            }

            fpgalog << "Creating light cache buffer, size: "
                    << dev::getFormattedMemory((double)m_epochContext.lightSize);
            m_light.clear();

            bool light_on_host = false;
            try
            {
                m_light.emplace_back(m_context[0], CL_MEM_READ_ONLY, m_epochContext.lightSize);
            }
            catch(cl::Error const& e)
            {
                if ((e.err() == CL_OUT_OF_RESOURCES) || (e.err() == CL_OUT_OF_HOST_MEMORY))
                {
                    // Ok, no room for light cache on GPU. Try allocating on host
                    clog(WarnChannel) << "No room on GPU, allocating light cache on host";
                    clog(WarnChannel) << "Generating DAG will take minutes instead of seconds";
                    light_on_host = true;
                }
                else
                    throw;
            }

            if(light_on_host)
            {
                m_light.emplace_back(m_context[0], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                    m_epochContext.lightSize);
                fpgalog << "WARNING: Generating DAG will take minutes, not seconds";
            }
            fpgalog << "Loading kernels...";

            m_searchKernel = cl::Kernel(program, "search", &err);
            m_dagKernel = cl::Kernel(program, "GenerateDAG", &err);

            m_queue[0].enqueueWriteBuffer(
                m_light[0], CL_TRUE, 0, m_epochContext.lightSize, m_epochContext.lightCache);


        }   
        catch(cl::Error const& e)
        {
            cwarn << "Creating DAG buffer failed";
            fpgalog << e.what() << "\n";
            pause(MinerPauseEnum::PauseDueToInitEpochError);
            return true;
        }
        
        //create buffer for header
        fpgalog << "Creating buffer for header.";
        
        m_header.clear();
        m_header.push_back(cl::Buffer(m_context[0], CL_MEM_WRITE_ONLY, sizeof(1)));

        


        auto dagTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startInit);
        fpgalog << dev::getFormattedMemory((double)m_epochContext.dagSize)
              << " of DAG data generated in "
              << dagTime.count() << " ms.";


    }
    catch(cl::Error const& e)
    {
        fpgalog << "OpenCL init failed"
                << e.what() << "\n";
        pause(MinerPauseEnum::PauseDueToInitEpochError);
        return false;
    }
    
    return true;
}

void FPGAMiner::kick_miner()
{   
    // TODO
    m_new_work_signal.notify_one();
}



struct SearchResults
{

    struct
    {
        uint32_t gid;
        uint32_t mix[8];
        uint32_t pad[7];    // pad to 16 words for easy indexing
    } rslt[c_maxSearchResults];
    
    uint32_t count;
    uint32_t hashCount;
    uint32_t abort;

};

void FPGAMiner::workLoop()
{

    uint32_t zerox3[3] = {0,0,0};
    uint64_t startNonce = 0;

    WorkPackage current;
    current.header = h256();
     
    if(!initDevice())
    {
        printf("\nFPGAMiner::initDevice()\tFAILED\n");
        return;
    }
    
    try
    {
        while(!shouldStop())
        {
            // Read results.
            volatile SearchResults results;

            if (m_queue.size())
            {
                // no need to read the abort flag.
                m_queue[0].enqueueReadBuffer(m_searchBuffer[0], CL_TRUE,
                    offsetof(SearchResults, count),
                    (m_settings.noExit ? 1 : 2) * sizeof(results.count), (void*)&results.count);
                
                if (results.count)
                {
                    if (results.count > c_maxSearchResults) {
                        results.count = c_maxSearchResults;
                    }

                    m_queue[0].enqueueReadBuffer(m_searchBuffer[0], CL_TRUE, 0,
                        results.count * sizeof(results.rslt[0]), (void*)&results);
                    // Reset search buffer if any solution found.
                    if (m_settings.noExit)
                        m_queue[0].enqueueWriteBuffer(m_searchBuffer[0], CL_FALSE,
                            offsetof(SearchResults, count), sizeof(results.count), zerox3);
                }

                // clean the solution count, hash count, and abort flag
                if (!m_settings.noExit)
                    m_queue[0].enqueueWriteBuffer(m_searchBuffer[0], CL_FALSE,
                        offsetof(SearchResults, count), sizeof(zerox3), zerox3);
            }
            else
                results.count = 0;

            
            
            // Wait for work or 3 seconds (whichever the first)
            const WorkPackage w = work();
            if (!w)
            {
                boost::system_time const timeout =
                    boost::get_system_time() + boost::posix_time::seconds(3);
                boost::mutex::scoped_lock l(x_work);
                m_new_work_signal.timed_wait(l, timeout);
                continue;
            }



            if (current.header != w.header)
            {

                if (current.epoch != w.epoch)
                {
                    m_abortqueue.clear();

                    if (!initEpoch())
                        break;  // This will simply exit the thread

                    m_abortqueue.push_back(cl::CommandQueue(m_context[0], m_device));
                }

                // Upper 64 bits of the boundary.
                const uint64_t target = (uint64_t)(u64)((u256)w.boundary >> 192);
                // assert(target > 0);

                startNonce = w.startNonce;

                // Update header constant buffer.
                m_queue[0].enqueueWriteBuffer(
                    m_header[0], CL_FALSE, 0, w.header.size, w.header.data());
                // zero the result count
                m_queue[0].enqueueWriteBuffer(m_searchBuffer[0], CL_FALSE,
                    offsetof(SearchResults, count),
                    m_settings.noExit ? sizeof(zerox3[0]) : sizeof(zerox3), zerox3);

                m_searchKernel.setArg(0, m_searchBuffer[0]);  // Supply output buffer to kernel.
                m_searchKernel.setArg(1, m_header[0]);        // Supply header buffer to kernel.
                m_searchKernel.setArg(2, m_dag[0]);           // Supply DAG buffer to kernel.
                m_searchKernel.setArg(3, m_dag[1]);           // Supply DAG buffer to kernel.
                m_searchKernel.setArg(4, m_dagItems);
                m_searchKernel.setArg(6, target);

#ifdef DEV_BUILD
                if (g_logOptions & LOG_SWITCH)
                    fpgalog << "Switch time: "
                          << std::chrono::duration_cast<std::chrono::microseconds>(
                                 std::chrono::steady_clock::now() - m_workSwitchStart)
                                 .count()
                          << " us.";
#endif
            }


            // Run the kernel.
            m_searchKernel.setArg(5, startNonce);
            m_queue[0].enqueueNDRangeKernel(
                m_searchKernel, cl::NullRange, m_settings.globalWorkSize, m_settings.localWorkSize);

            if (results.count)
            {
                // Report results while the kernel is running.
                for (uint32_t i = 0; i < results.count; i++)
                {
                    uint64_t nonce = current.startNonce + results.rslt[i].gid;
                    if (nonce != m_lastNonce)
                    {
                        m_lastNonce = nonce;
                        h256 mix;
                        memcpy(mix.data(), (char*)results.rslt[i].mix, sizeof(results.rslt[i].mix));

                        Farm::f().submitProof(Solution{
                            nonce, mix, current, std::chrono::steady_clock::now(), m_index});
                        fpgalog << EthWhite << "Job: " << current.header.abridged() << " Sol: 0x"
                              << toHex(nonce) << EthReset;
                    }
                }
            }

            current = w;  // kernel now processing newest work
            current.startNonce = startNonce;
            // Increase start nonce for following kernel execution.
            startNonce += m_settings.globalWorkSize;
            // Report hash count
            if (m_settings.noExit)
                updateHashRate(m_settings.globalWorkSize, 1);
            else
                updateHashRate(m_settings.localWorkSize, results.hashCount);

        }
    
    
        if(m_queue.size())
            m_queue[0].finish();

        clear_buffer();
    
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}



static char nibbleToChar(unsigned nibble)
{
    return (char) ((nibble >= 10 ? 'a'-10 : '0') + nibble);
}
static uint8_t charToNibble(char chr)
{
	if (chr >= '0' && chr <= '9')
	{
		return (uint8_t) (chr - '0');
	}
	if (chr >= 'a' && chr <= 'z')
	{
		return (uint8_t) (chr - 'a' + 10);
	}
	if (chr >= 'A' && chr <= 'Z')
	{
		return (uint8_t) (chr - 'A' + 10);
	}
	return 0;
}
static std::vector<uint8_t> hexStringToBytes(char const* str)
{
	std::vector<uint8_t> bytes(strlen(str) >> 1);
	for (unsigned i = 0; i != bytes.size(); ++i)
	{
		bytes[i] = charToNibble(str[i*2 | 0]) << 4;
		bytes[i] |= charToNibble(str[i*2 | 1]);
	}
	return bytes;
}
static std::string bytesToHexString(uint8_t const* bytes, unsigned size)
{
	std::string str;
	for (unsigned i = 0; i != size; ++i)
	{
		str += nibbleToChar(bytes[i] >> 4);
		str += nibbleToChar(bytes[i] & 0xf);
	}
	return str;
}







/*
This file is part of ethminer-changes

*/

#pragma once

// #include <fstream>

// #include <experimental/xrt_kernel.h>
// #include <experimental/xrt_aie.h>

#include <libdevcore/Worker.h>
#include <libethcore/EthashAux.h>
#include <libethcore/Miner.h>

// #include <CL/cl.hpp>
#pragma GCC diagnostic push
#if __GNUC__ >= 6
#pragma GCC diagnostic ignored "-Wignored-attributes"
#endif
#pragma GCC diagnostic ignored "-Wmissing-braces"
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS true
#define CL_HPP_ENABLE_EXCEPTIONS true
#define CL_HPP_CL_1_2_DEFAULT_BUILD true
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#include "xcl2.hpp"
#include <CL/cl_ext_xilinx.h>

#pragma GCC diagnostic pop

// macOS OpenCL fix:
#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV 0x4000
#endif

#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV 0x4001
#endif

#define DATA_SIZE 4096

namespace dev
{
namespace eth
{


class FPGAMiner : public Miner
{
public:
    FPGAMiner(unsigned _index, FPGASettings _settings, DeviceDescriptor& _device);
    ~FPGAMiner() override;
    
    static void enumDevices(std::map<string, DeviceDescriptor>& _DevicesCollection);

    // void search(const dev::eth::WorkPackage& w);

public:

    static char nibbleToChar(unsigned nibble);
    static uint8_t charToNibble(char chr);
    static std::vector<uint8_t> hexStringToBytes(char const* str);
    static std::string bytesToHexString(uint8_t const* bytes, unsigned size);
    
protected:
    bool initDevice() override;
    bool initEpoch_internal() override;
    void kick_miner() override;

private:

    void workLoop() override;

    vector<cl::Context> m_context;
    vector<cl::CommandQueue> m_queue;
    vector<cl::CommandQueue> m_abortqueue;
    cl::Kernel m_searchKernel;
    cl::Kernel m_dagKernel;
    cl::Device m_device;

    vector<cl::Buffer> m_dag;
    vector<cl::Buffer> m_light;
    vector<cl::Buffer> m_header;
    vector<cl::Buffer> m_searchBuffer;

    void clear_buffer() {
        m_dag.clear();
        m_light.clear();
        m_header.clear();
        m_searchBuffer.clear();
        m_queue.clear();
        m_context.clear();
        m_abortqueue.clear();
    }
    
    FPGASettings m_settings;

    unsigned m_dagItems = 0;
    uint64_t m_lastNonce = 0; // make this a global memory on card

};


}
}
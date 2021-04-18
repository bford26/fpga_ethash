#! /bin/sh

# setup XILINX_VITIS and XILINX_VIVADO vars
source /tools/Xilinx/Vitis/2020.2/settings64.sh
# setup XILINX_XRT
source /opt/xilinx/xrt/setup.sh

export PLATFORM_REPO_PATHS=/opt/xilinx/platforms
# only on ubunut distributions
export LIBRARY_PATH=/usr/lib/x86_64-linux-gnu

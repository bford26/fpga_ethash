# setup XILINX_VITIS and XILINX_VIVADO vars
source /tools/Xilinx/Vitis/2020.2/settings64.sh
# setup XILINX_XRT
source /opt/xilinx/xrt/setup.sh

export PLATFORM_REPO_PATHS=/opt/xilinx/platforms
# only on ubunut distributions
export LIBRARY_PATH=/usr/lib/x86_64-linux-gnu

echo ""
echo "Xilinx ENV Setup!"
echo ""

# create simulation json
# emconfigutil --platform xilinx_u280_xdma_201920_3 --nd 1

# compile kernel object
# v++ -c -t sw_emu --platform xilinx_u280_xdma_201920_3 --config ../../../libethash-fpga/configs/u280.cfg -k kernel_func -I../../../libethash-fpga/ ../../../libethash-fpga/kernel_func.cpp -o ../../../libethash-fpga/kernel_func.xo

# compile xclbin object
# v++ -l -t sw_emu --platform xilinx_u280_xdma_201920_3 --config ../../../libethash-fpga/configs/u280.cfg ../../../libethash-fpga/kernel_func.xo -o /out/build/kernel_func.xclbin

echo ""
echo "Finished Xilinx Compiles.."
echo ""
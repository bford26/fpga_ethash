echo ""
echo "Making emconfig JSON.."
echo ""
# emconfigutil --platform xilinx_u280_xdma_201920_3 --od ./build/ethminer

export XCL_EMULATION_MODE=sw_emu

echo ""
echo "Starting Simulation..."
echo ""
./build/ethminer/ethminer1 --fpga --list-devices -P stratum1+ssl://0x26D632c9E204848145De97f813f2E04eE6Dce554.MainRig@us1.ethermine.org:5555 -P stratum1+ssl://0x26D632c9E204848145De97f813f2E04eE6Dce554.MainRig@us2.ethermine.org:5555


 


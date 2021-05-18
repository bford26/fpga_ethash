

# ssl pools
cd ./build && make && ./ethminer/ethminer -Z 0
# cd ./build && make && ./ethminer/ethminer --fpga -P stratum1+ssl://0x26D632c9E204848145De97f813f2E04eE6Dce554.MainRig@us1.ethermine.org:5555 -P stratum1+ssl://0x26D632c9E204848145De97f813f2E04eE6Dce554.MainRig@us2.ethermine.org:5555
# ./build/ethminer/ethminer1

# echo ""
# echo "Starting Mining Program..."
# echo ""
# echo ""

# non ssl pools
# cd ./build && make && ./ethminer/ethminer1 --fpga --list-devices -P stratum1+tcp://0x26D632c9E204848145De97f813f2E04eE6Dce554.MainRig@us1.ethermine.org:4444 -P stratum1+tcp://0x26D632c9E204848145De97f813f2E04eE6Dce554.MainRig@us2.ethermine.org:4444


#pragma once

// #include <string>

class Miner
{

public:
    Miner(unsigned _index);
    ~Miner();

};


class FPGAMiner : public Miner
{

public:
    FPGAMiner(unsigned _index);
    ~FPGAMiner();

};
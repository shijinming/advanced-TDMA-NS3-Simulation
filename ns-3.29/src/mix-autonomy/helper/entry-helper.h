#ifndef MIX_AUTONOMY_ENTRY_HELPER
#define MIX_AUTONOMY_ENTRY_HELPER

#include "ns3/sim-config.h"

namespace ns3 {

class SimulationEntry 
{
public:
    SimulationEntry ();
    void Simulate (int argc, char **argv);
private:
    SimulationConfig &config;
};

}

#endif
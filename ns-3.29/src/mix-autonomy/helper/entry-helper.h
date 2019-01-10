#ifndef MIX_AUTONOMY_ENTRY_HELPER
#define MIX_AUTONOMY_ENTRY_HELPER

#include "ns3/mobility-module.h"
#include "ns3/sim-config.h"

namespace ns3 {

class SimulationEntry 
{
public:
    SimulationEntry ();
    void Simulate (int argc, char **argv);
private:
    SimulationConfig &config;

    /**
     * 配置tracing监听系统
     */ 
    void ConfigureTracing ();

    /**
     * 配置应用信息
     */
    void ConfigureApplication ();
    /**
     * 在Mobility状态发生变化时调用这个回调
     */
    void CourseChange (std::string context, Ptr<const MobilityModel> mobility);

    /**
     * 一个周期性（每秒）调用的函数，可以在这里面输出统计信息
     */
    void PeriodicCheck ();
};

}

#endif
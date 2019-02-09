#ifndef MIX_AUTONOMY_ENTRY_HELPER
#define MIX_AUTONOMY_ENTRY_HELPER

#include "ns3/mobility-module.h"
#include "ns3/sim-config.h"

namespace ns3 {

class SimulationEntry 
{
public:
    SimulationEntry ();
    virtual void Simulate (int argc, char **argv);

protected:
    SimulationConfig &config;

    double txPower = 35;
    bool doValidate = true;

    /**
     * @brief 一个钩子函数，可以在这里面覆盖config中的设置
     */
    virtual void OverrideDefaultConfig () {};

    /**
     * @brief 配置运动数据
     */
    virtual void LoadMobilityData ();

    /**
     * @brief 配置tracing监听系统
     */ 
    virtual void ConfigureTracing () {};

    /**
     * @brief 配置应用信息
     */
    virtual void ConfigureApplication () = 0;

    /**
     * @brief 一个周期性（每秒）调用的函数，可以在这里面输出统计信息
     */
    virtual void PeriodicCheck ();
};

}

#endif
#include "sim-config.h"
#include <sys/stat.h>

namespace ns3
{

SimulationConfig &
SimulationConfig::Default()
{
    static SimulationConfig config = SimulationConfig();
    return config;
}

void SimulationConfig::ParceCommandLineArguments(int argc, char **argv, bool doValidate)
{
    CommandLine cmd;
    ConfigureCommandLineParams(cmd);
    cmd.Parse(argc, argv);
    if (doValidate)
        Validate();
}

void SimulationConfig::ConfigureCommandLineParams(CommandLine &cmd)
{
    cmd.AddValue("task-id", "ID of this task", taskId);
    cmd.AddValue("nnodes", "Number of nodes", nNodes);
    cmd.AddValue("ap-num", "Number of autonomous vehicles", apNum);
    cmd.AddValue("port", "Port number used by socket", socketPort);
    cmd.AddValue("sim-time", "Simulation duration", simTime);
    cmd.AddValue("phy-mode", "Working mode of phy layer", phyMode);
    cmd.AddValue("mobility", "Path to mobility file", mobilityInput);
    cmd.AddValue("packet-trace", "Path to packet trace file", packetTraceFile);
    cmd.AddValue("event-trace", "Path to event trace file", eventTraceFile);
    cmd.AddValue("throughput-trace", "Path to throughput file", throughputTraceFile);
    cmd.AddValue("mobility-trace", "Path to mobility file", mobilityTraceFile);
    cmd.AddValue("start-time", "Path to start time file", startTimeFile);
    cmd.AddValue("cw-min", "Middle minimum contention window", cwMin);
    cmd.AddValue("cw-max", "Middle maximum contention window", cwMax);
    cmd.AddValue("send-num", "number of packets to send in a period", sendNum);
    cmd.AddValue("reference", "simulation for reference", reference);
}

void SimulationConfig::PrintConfiguration()
{

#define SHOW_CONF(x) LOG_UNCOND(#x << ": " << x)
    LOG_UNCOND("==================== SIMULATION SETTINGS");
    SHOW_CONF(taskId);
    SHOW_CONF(nNodes);
    SHOW_CONF(simTime);
    SHOW_CONF(apNum);
    SHOW_CONF(socketPort);
    SHOW_CONF(phyMode);
    SHOW_CONF(mobilityInput);
    SHOW_CONF(packetTraceFile);
    SHOW_CONF(eventTraceFile);
    SHOW_CONF(throughputTraceFile);
    SHOW_CONF(mobilityTraceFile);
    SHOW_CONF(startTimeFile);
    SHOW_CONF(cwMin);
    SHOW_CONF(cwMax);
    SHOW_CONF(sendNum);
    SHOW_CONF(reference);
    LOG_UNCOND("====================");
#undef SHOW_CONF
}

void SimulationConfig::Validate()
{
    struct stat buf;
    if (stat(mobilityInput.c_str(), &buf) != 0)
    {
        LOG_UNCOND("Cannot open mobility input file: " << mobilityInput);
        exit(1);
    }
}

} // namespace ns3
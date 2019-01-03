#include <sstream>

#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/network-module.h"
#include "ns3/wave-module.h"
#include "ns3/mobility-module.h"

#include "entry-helper.h"

namespace ns3
{

SimulationEntry::SimulationEntry ()
    : config(SimulationConfig::Default ())
{
}

void SimulationEntry::Simulate (int argc, char **argv)
{
  LOG_UNCOND ("Simulation Start");
  config.ParceCommandLineArguments (argc, argv);
  config.PrintConfiguration ();

  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (config.phyMode));

  NodeContainer nodes;
  nodes.Create (config.nNodes);

  // Physical layer
  YansWifiPhyHelper wifiPhy = YansWavePhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
  wifiPhy.Set ("TxPowerStart", DoubleValue (35));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (35));
  // Mac layer
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  // 802.11p protocol
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                     "DataMode", StringValue (config.phyMode),
                                     "ControlMode", StringValue (config.phyMode));

  NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, nodes);

  Ns2MobilityHelper ns2Helper = Ns2MobilityHelper (config.mobilityInput);
  ns2Helper.Install ();

  ConfigureTracing ();
  ConfigureApplication ();

  PeriodicCheck ();
}

void SimulationEntry::ConfigureTracing()
{
  Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
                  MakeCallback(&SimulationEntry::CourseChange, this));
}

// https://www.cnblogs.com/dfcao/p/cpp-FAQ-split.html
static void SplitString(const std::string &s, std::vector<std::string> &v, const std::string &c)
{
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while (std::string::npos != pos2)
  {
    v.push_back(s.substr(pos1, pos2 - pos1));

    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if (pos1 != s.length())
    v.push_back(s.substr(pos1));
}

void SimulationEntry::CourseChange(std::string context, Ptr<const MobilityModel> mobility)
{
  std::vector<std::string> elements;
  SplitString(context, elements, "/");
  if (elements.size() < 3)
  {
    LOG_UNCOND("Conext format error[0]: " << context);
    exit(1);
  }

  uint32_t iNodeId = std::stoul(elements[2]);
  std::istringstream iss (elements[2]);
  iss >> iNodeId;
  // TODO: activate this node 
  LOG_UNCOND ("Node " << iNodeId << " starts to move!");

  // Disconnect this trace callback
  Config::Disconnect (context, MakeCallback(&SimulationEntry::CourseChange, this));
}

void
SimulationEntry::PeriodicCheck () {

  Simulator::Schedule (Seconds (1),
    &SimulationEntry::PeriodicCheck, this);
}

} // namespace ns3
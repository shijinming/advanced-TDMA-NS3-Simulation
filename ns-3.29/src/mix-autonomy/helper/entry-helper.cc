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

  LOG_UNCOND ("Load mobility data");
  Ns2MobilityHelper ns2Helper = Ns2MobilityHelper (config.mobilityInput);
  ns2Helper.Install ();
  LOG_UNCOND ("Load finished");

  ConfigureTracing ();
  ConfigureApplication ();

  PeriodicCheck ();
  Simulator::Stop (Seconds (config.simTime));
  Simulator::Run ();
  Simulator::Destroy ();
}

void SimulationEntry::ConfigureTracing()
{
  LOG_UNCOND ("Config tracing");
}

void
SimulationEntry::ConfigureApplication () {
  
}

void
SimulationEntry::PeriodicCheck () {
  LOG_UNCOND ("Tick" << Simulator::Now ().GetMilliSeconds () << "ms");
  Simulator::Schedule (Seconds (1),
    &SimulationEntry::PeriodicCheck, this);
}

} // namespace ns3
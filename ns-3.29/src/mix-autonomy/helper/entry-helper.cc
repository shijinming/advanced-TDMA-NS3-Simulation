#include <sstream>

#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
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
  config.ParceCommandLineArguments (argc, argv, doValidate);
  OverrideDefaultConfig ();
  config.PrintConfiguration ();

  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (config.phyMode));

  NodeContainer nodes;
  nodes.Create (config.nNodes);

  // Physical layer
  auto wifiPhy = YansWavePhyHelper::Default ();
  auto wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
  wifiPhy.Set ("TxPowerStart", DoubleValue (txPower));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (txPower));  // txPower: default valude is 35
  // Mac layer
  auto wifi80211pMac = NqosWaveMacHelper::Default ();
  // 802.11p protocol
  auto wifi80211p = Wifi80211pHelper::Default ();
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                     "DataMode", StringValue (config.phyMode),
                                     "ControlMode", StringValue (config.phyMode));

  auto devices = wifi80211p.Install (wifiPhy, wifi80211pMac, nodes);

  InternetStackHelper internet;
  internet.Install (nodes);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.0.0", "255.255.0.0");
  auto interfaces = ipv4.Assign (devices);

  LoadMobilityData ();
  ConfigureTracing ();
  ConfigureApplication ();
  PeriodicCheck ();

  Simulator::Stop (Seconds (config.simTime));
  Simulator::Run ();
  Simulator::Destroy ();
}


void
SimulationEntry::PeriodicCheck () {
  LOG_UNCOND ("Tick" << Simulator::Now ().GetMilliSeconds () << "ms");
  Simulator::Schedule (Seconds (1),
    &SimulationEntry::PeriodicCheck, this);
}

void
SimulationEntry::LoadMobilityData ()
{
  LOG_UNCOND ("Load mobility data");
  Ns2MobilityHelper ns2Helper = Ns2MobilityHelper (config.mobilityInput);
  ns2Helper.Install ();
  LOG_UNCOND ("Load finished");
}

} // namespace ns3
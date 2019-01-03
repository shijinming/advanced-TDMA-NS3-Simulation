#include "entry-helper.h"

namespace ns3 {

SimulationEntry::SimulationEntry ()
  : config (SimulationConfig::Default ())
{

}

void
SimulationConfig::Simulate (int argc, char **argv) 
{
    LOG_UNCOND ("Simulation Start");
    config.ParseCommandLineArguments (argc, argv);
    config.PrintConfiguration ();

    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (config.phyMode));

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
}



}
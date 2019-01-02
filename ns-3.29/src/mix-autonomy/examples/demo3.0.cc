/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EnhancedWifimodeVehicle");

// ===========================================================================
//          AP
//         node 0                 node 1               node2                  node3                 node4
//   +----------------+    +----------------+    +----------------+    +----------------+     +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |    |    ns-3 TCP    |    |    ns-3 TCP    |     |    ns-3 TCP    |
//   +----------------+    +----------------+    +----------------+    +----------------+     +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |    |    10.1.1.3    |    |    10.1.1.4    |     |    10.1.1.5    |
//   +----------------+    +----------------+    +----------------+    +----------------+     +----------------+
//   |      wifi      |    |      wifi      |    |      wifi      |    |      wifi      |     |      wifi      |
//   +----------------+    +----------------+    +----------------+    +----------------+     +----------------+
//           |       1Mbps        |      ...                    ...
//
// ===========================================================================
//
class MyApp : public Application 
{
public:

  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0), 
    m_peer (), 
    m_packetSize (0), 
    m_nPackets (0), 
    m_dataRate (0), 
    m_sendEvent (), 
    m_running (false), 
    m_packetsSent (0)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void 
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void 
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void 
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

static void
CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
}

static void
RxDrop (Ptr<const Packet> p)
{
  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
}

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi = 4;
  //bool tracing = false;
    
  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc, argv);
  
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }
    
 /*
  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
  */

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
    
  NodeContainer wifiApNode;
  wifiApNode.Create (1);
    
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
    
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
    
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
    
  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);
    
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
    
  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);
  
  //add packet loss on purpose
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  staDevices.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  MobilityHelper mobility;
    
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
    
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);
    
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;
  
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer wifiInterfaces;
  address.Assign (staDevices);
  wifiInterfaces = address.Assign (apDevices);

  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (wifiInterfaces.GetAddress (0), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (wifiApNode);
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20.));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (wifiStaNodes.Get (0), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));

  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 1040, 1000, DataRate ("1Mbps"));
  wifiStaNodes.Get (0)->AddApplication (app);
  wifiStaNodes.Get (1)->AddApplication (app);
  wifiStaNodes.Get (2)->AddApplication (app);
  wifiStaNodes.Get (3)->AddApplication (app);
  
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20.));

  apDevices.Get (0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));

  Simulator::Stop (Seconds (20));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}


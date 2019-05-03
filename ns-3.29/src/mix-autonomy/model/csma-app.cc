/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/internet-module.h"
#include "ns3/wave-module.h"
#include<stdlib.h>
#include "csma-app.h"
#include "ap-leader.h"

namespace ns3
{
  
NS_LOG_COMPONENT_DEFINE("CSMAApplication");
NS_OBJECT_ENSURE_REGISTERED(CSMAApplication);

TypeId
CSMAApplication::GetTypeId()
{
  static TypeId tid = TypeId("ns3::CSMAApplication")
                          .SetParent<Application>()
                          .AddConstructor<CSMAApplication>()
													.AddTraceSource("Tx", "A new packet is created and is sent",
                                          MakeTraceSourceAccessor(&CSMAApplication::txTrace),
                                          "ns3::Packet::AddressTraceCallback")
                          .AddTraceSource("Rx", "A new packet is received by sink",
                                          MakeTraceSourceAccessor(&CSMAApplication::rxTrace),
                                          "ns3::ThreeLayerHelper::TracedCallback");
  return tid;
}

CSMAApplication::CSMAApplication()
		:  config(SimulationConfig::Default())
{
  m_isMiddle = false;
}

CSMAApplication::~CSMAApplication()
{
}

void
CSMAApplication::StartApplication(void)
{
	std::cout << GetNode()->GetId() << " starts at " << Simulator::Now() << std::endl;
	CreateSocket();
	SendPacket();
}

void 
CSMAApplication::StopApplication(void)
{
	sendEvent.Cancel();
}

void
CSMAApplication::DoDispose()
{
	socket = NULL;
  sink = NULL;
}

void
CSMAApplication::DoInitialize()
{
	std::ifstream f(config.startTimeFile);
  if (!f.is_open())
    std::cout << "start time file is not open!" << std::endl;
  float t = 0;
  for (uint32_t i = 0; i <= GetNode()->GetId(); i++)
    f >> t;
  m_startTime = Seconds(t) + MicroSeconds(rand()%50000);
  m_stopTime = Seconds(config.simTime);

  Ptr<WaveNetDevice> device = DynamicCast<WaveNetDevice> (GetNode ()->GetDevice (0));
  Ptr<WifiPhy> phy = device->GetPhy (0);
  phy->TraceConnectWithoutContext("PhyTxBegin", MakeCallback(&CSMAApplication::WifiPhyTxBeginTrace, this));
  m_gi = device->GetChannelCoordinator()->GetGuardInterval();
  m_cchi = device->GetChannelCoordinator()->GetCchInterval();
  m_schi = device->GetChannelCoordinator()->GetSchInterval();
  m_synci = device->GetChannelCoordinator()->GetSyncInterval();
	Application::DoInitialize();
}

void
CSMAApplication::CreateSocket()
{
	auto broadcastAddr = InetSocketAddress(Ipv4Address("255.255.255.255"), config.socketPort);
  socketTid = UdpSocketFactory::GetTypeId();
  if (!socket)
  {
    socket = Socket::CreateSocket(GetNode(), socketTid);
    if (socket->Bind())
    {
      LOG_UNCOND("Fatal Error: Fail to bind socket");
      exit(1);
    }
    socket->Connect(broadcastAddr);
    socket->SetAllowBroadcast(true);
    socket->ShutdownRecv();
  }
  if (!sink)
  {
    sink = Socket::CreateSocket(GetNode(), socketTid);
    if (sink->Bind(Address(InetSocketAddress(Ipv4Address::GetAny(), config.socketPort))))
    {
      LOG_UNCOND("Fatal Error: Fail to bind socket");
      exit(1);
    }
    sink->Listen();
    sink->ShutdownSend();
    sink->SetRecvCallback(MakeCallback(&CSMAApplication::OnReceivePacket, this));
  }
}

void 
CSMAApplication::OnReceivePacket(Ptr<Socket> socket)
{
	Ptr<Packet> pkt;
  Address srcAddr;
  while ((pkt = sink->RecvFrom(srcAddr)))
  {
    InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom(srcAddr);
    Address addr = inetAddr.GetIpv4();
    rxTrace(pkt, this, addr);
  }
}
void 
CSMAApplication::DoSendPacket(Ptr<Packet> pkt)
{
  socket->Send(pkt);
  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
  txTrace(pkt, ipv4->GetAddress(1, 0).GetLocal());
}

void
CSMAApplication::SendPacket(void)
{
  bool curChannel=(Simulator::Now().GetMilliSeconds()%50 < 50);
  Ptr<Packet> pktToSend;
  if (curChannel)
  {
    if(!txqCCH.empty())
    {
      pktToSend = txqCCH.front();
      txqCCH.pop();
    }
  }
  else
  {
    if(!txqSCH.empty())
    {
      pktToSend = txqSCH.front();
      txqSCH.pop();
    }
  }
	DoSendPacket(pktToSend);
}

void 
CSMAApplication::WifiPhyTxBeginTrace(Ptr<const Packet> p)
{
  SendPacket();
}

void CSMAApplication::generateTraffic(void)
{
  if(m_isMiddle)
  {

  }
  else
  {
    /* code */
  }
  
}

bool 
CSMAApplication::IsAPApplicationInstalled(Ptr<Node> node)
{
  uint32_t nApps = node->GetNApplications();
  for (uint32_t idx = 0; idx < nApps; idx++)
  {
    Ptr<Application> app = node->GetApplication(idx);
    if (dynamic_cast<APFollower *>(PeekPointer(app)) || dynamic_cast<APLeader *>(PeekPointer(app)))
    {
      return true;
    }
  }
  return false;
}

void 
CSMAApplication::ReceivePacket(Ptr<Packet> pkt, Address &srcAddr)
{
  InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom(srcAddr);
  Ipv4Address addr = inetAddr.GetIpv4();
  // 获取发送该数据包的节点
  Ptr<Node> node = GetNodeFromAddress(addr);
  if (IsAPApplicationInstalled(node))
  {
    lastTimeRecAP = Simulator::Now();
    m_isMiddle = true;
    ReceiveFromAP(pkt, node);
  }
  else
  {
    if(Simulator::Now()-lastTimeRecAP > MilliSeconds(200))
      m_isMiddle = false;
  }
}

Ptr<Node>
CSMAApplication::GetNodeFromAddress(Ipv4Address &address)
{
  for (NodeList::Iterator n = NodeList::Begin();
       n != NodeList::End(); n++)
  {
    Ptr<Node> node = *n;
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    NS_ASSERT(ipv4);
    if (ipv4->GetInterfaceForAddress(address) != -1)
    {
      return node;
    }
  }
  return NULL;
}

void 
CSMAApplication::ReceiveFromAP(Ptr<Packet> pkt, Ptr<Node> node)
{
  
}

void 
CSMAApplication::SwitchSCH()
{
  if(!m_isMiddle)
    return;
  SchInfo schInfo;
  if(Simulator::Now().GetMilliSeconds()%200<100)
  {
    schInfo = SchInfo (SCH1, false, EXTENDED_ALTERNATING);
  }
  else
  {
    schInfo = SchInfo (SCH2, false, EXTENDED_ALTERNATING);
  }
  Ptr<WaveNetDevice>  device = DynamicCast<WaveNetDevice> (GetNode()->GetDevice(0));
  Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, device, schInfo);
}



}
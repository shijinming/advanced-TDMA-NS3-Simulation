/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/internet-module.h"
#include<stdlib.h>
#include "csma-app.h"

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
	Ptr<Packet> pktToSend = Create<Packet>(1000);
	DoSendPacket(pktToSend);
	Time t = MilliSeconds(50);
  sendEvent = Simulator::Schedule(t, &CSMAApplication::SendPacket, this);
}

}
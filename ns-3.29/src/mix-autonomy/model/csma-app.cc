/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/internet-module.h"
#include "ns3/wave-module.h"
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
                          .AddAttribute("VehicleType", "Vehicle type",
                                        UintegerValue(0),
                                        MakeUintegerAccessor(&CSMAApplication::m_type),
                                        MakeUintegerChecker<uint16_t>())
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
  m_device->SetReceiveCallback (MakeCallback(&CSMAApplication::ReceivePacket, this));
  startTxCCH = MilliSeconds(0);
  startTxSCH = MilliSeconds(0);
  Time temp = MicroSeconds(Simulator::Now().GetMicroSeconds()%m_synci.GetMicroSeconds());
  Time start = (temp<=m_gi)?(m_gi-temp):(m_gi+m_synci-temp);
  Simulator::Schedule (start, &CSMAApplication::StartCCH, this);
  GenerateTraffic();
}

void 
CSMAApplication::StopApplication(void)
{
}

void
CSMAApplication::DoDispose()
{
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
  m_startTime = Seconds(t);
  m_stopTime = Seconds(config.simTime);
  m_isMiddle = false;
  m_device = DynamicCast<WaveNetDevice> (GetNode ()->GetDevice (0));
  Ptr<WifiPhy> phy = m_device->GetPhy (0);
  phy->TraceConnectWithoutContext("PhyTxBegin", MakeCallback(&CSMAApplication::WifiPhyTxBeginTrace, this));
  m_gi = m_device->GetChannelCoordinator()->GetGuardInterval();
  m_cchi = m_device->GetChannelCoordinator()->GetCchInterval();
  m_schi = m_device->GetChannelCoordinator()->GetSchInterval();
  m_synci = m_device->GetChannelCoordinator()->GetSyncInterval();
  m_SCH = SCH2;
	Application::DoInitialize();
}

void 
CSMAApplication::DoSendPacket(Ptr<Packet> pkt, uint32_t channel)
{
  m_device->SendX(pkt, Mac48Address::GetBroadcast (),config.socketPort, TxInfo(channel));
  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
  txTrace(pkt, ipv4->GetAddress(1, 0).GetLocal());
}

void
CSMAApplication::SendPacket(void)
{
  Ptr<WaveNetDevice>  device = DynamicCast<WaveNetDevice> (GetNode()->GetDevice(0));
  Time current =MicroSeconds(Simulator::Now().GetMicroSeconds()%m_synci.GetMicroSeconds());
  if(current < m_cchi + m_gi)
    return;
  if(Simulator::Now().GetMicroSeconds()%(2*m_synci.GetMicroSeconds()) < m_synci.GetMicroSeconds())
  {
    if(m_type>0 && current > m_cchi + m_gi + startTxSCH + m_durationSCH)
      return;
  }
  Ptr<Packet> pktToSend;
  // std::cout<<"txq length:"<<txq.size()<<std::endl;
  if(!txq.empty())
  {
    pktToSend = txq.front();
    txq.pop();
  }
	if(pktToSend!=NULL)
    DoSendPacket(pktToSend, m_SCH);
}

void 
CSMAApplication::WifiPhyTxBeginTrace(Ptr<const Packet> p)
{
  SendPacket();
}

void CSMAApplication::GenerateTraffic(void)
{
  for (int i=0;i<rand()%config.trafficSize;i++)
  {
    Ptr<Packet> pkt = Create<Packet> (200);
    txq.push(pkt);
  }
  Simulator::Schedule(MilliSeconds(rand()%500), &CSMAApplication::GenerateTraffic, this);
}

int
CSMAApplication::GetVehicleType ()
{
  return m_type;
}

bool 
CSMAApplication::ReceivePacket(Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &srcAddr)
{
  rxTrace(pkt, this, srcAddr);
  Ptr<Node> node = GetNodeFromAddress(srcAddr);
  Ptr<CSMAApplication> app = DynamicCast<CSMAApplication> (node->GetApplication(0));
  if (app->GetVehicleType()>0)
  {
    lastTimeRecAP = Simulator::Now();
    m_isMiddle = true;
    startTxCCH = config.apNum * config.slotSize;
    ReceiveFromAP(pkt, app->GetVehicleType());
  }
  else
  {
    if(Simulator::Now()-lastTimeRecAP > 2*m_synci)
      m_isMiddle = false;
  }
  return true;
}

Ptr<Node>
CSMAApplication::GetNodeFromAddress(const Address &address)
{
  for (NodeList::Iterator n = NodeList::Begin();
       n != NodeList::End(); n++)
  {
    Ptr<Node> node = *n;
    if (node->GetDevice(0)->GetAddress()==address)
    {
      return node;
    }
  }
  return NULL;
}

void 
CSMAApplication::ReceiveFromAP(Ptr<const Packet> pkt, uint16_t type)
{
  if(type==2)
  {
    if(Simulator::Now().GetMicroSeconds()%m_synci.GetMicroSeconds()<(m_cchi+m_gi).GetMicroSeconds())
    {
      PacketHeader pHeader;
      pkt->PeekHeader(pHeader);
      startTxCCH = config.slotSize*config.apNum;
      startTxSCH = config.slotSize*pHeader.GetSCHSlotNum();
    }
  }
}

void 
CSMAApplication::StartCCH()
{
  Ptr<Packet> pkt = Create<Packet> (200);
  Simulator::Schedule (startTxCCH + MicroSeconds (rand()%1000), &CSMAApplication::DoSendPacket, this, pkt, CCH);
  ChangeSCH();
  Time wait = m_cchi +m_gi - MicroSeconds (Simulator::Now().GetMicroSeconds()%m_synci.GetMicroSeconds());
  if(Simulator::Now().GetMicroSeconds()%(2*m_synci.GetMicroSeconds()) < m_synci.GetMicroSeconds() && m_isMiddle)
    Simulator::Schedule (startTxSCH + wait + MicroSeconds (rand()%1000), &CSMAApplication::SendPacket, this);
  else
    Simulator::Schedule (wait + MicroSeconds (rand()%1000), &CSMAApplication::SendPacket, this);
  Simulator::Schedule (m_synci, &CSMAApplication::StartCCH, this);
}

void 
CSMAApplication::ChangeSCH()
{
  if(Simulator::Now().GetMicroSeconds()%(2*m_synci.GetMicroSeconds()) < m_synci.GetMicroSeconds() && m_isMiddle)
  {
    m_SCH = SCH1;
  }
  else
  {
    m_SCH = SCH2;
  }
  SchInfo schInfo = SchInfo (m_SCH, false, EXTENDED_ALTERNATING);
  Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, m_device, schInfo);
}

}
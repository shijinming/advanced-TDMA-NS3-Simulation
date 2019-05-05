/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ap-app.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("APApplication");
NS_OBJECT_ENSURE_REGISTERED(APApplication);

TypeId
APApplication::GetTypeId()
{
  static TypeId tid = TypeId("ns3::APApplication")
                          .SetParent<CSMAApplication>()
                          .AddConstructor<APApplication>();
  return tid;
}

APApplication::APApplication()
{
}

APApplication::~APApplication()
{
}

void
APApplication::StartApplication()
{
  std::cout << GetNode()->GetId() << " starts at " << Simulator::Now() << std::endl;
  m_device->SetReceiveCallback (MakeCallback(&CSMAApplication::ReceivePacket, this));
  startTxCCH = (config.apNum-1-GetNode()->GetId())*config.slotSize;
  startTxSCH = MilliSeconds(0);
  Time temp = MicroSeconds(Simulator::Now().GetMicroSeconds()%m_synci.GetMicroSeconds());
  Time start = (temp<=m_gi)?(m_gi-temp):(m_gi+m_synci-temp);
  Simulator::Schedule (start, &APApplication::StartCCH, this);
  GenerateTraffic();
}

void
APApplication::DoInitialize()
{
  CSMAApplication::DoInitialize();
  m_SCH = SCH1;
}

bool
APApplication::ReceivePacket(Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &srcAddr)
{
  rxTrace(pkt, this, srcAddr);
  Ptr<Node> node = GetNodeFromAddress(srcAddr);
  Ptr<CSMAApplication> app = DynamicCast<CSMAApplication> (node->GetApplication(0));
  if (app->GetVehicleType()>0)
  {
    Time current =MicroSeconds(Simulator::Now().GetMicroSeconds()%m_synci.GetMicroSeconds());
    if(current < m_cchi+m_gi)
      ReceiveFromAP(pkt, app->GetVehicleType());
  }
  return true;
}

void
APApplication::ReceiveFromAP(Ptr<const Packet> pkt, uint16_t type)
{
  if(Simulator::Now().GetMicroSeconds()%m_synci.GetMicroSeconds()>(m_cchi+m_gi).GetMicroSeconds())
    return;
  PacketHeader pHeader;
  pkt->PeekHeader(pHeader);
  if(m_type==1)
  {
    if(!pHeader.GetIsLeader())
      return;
    hdrSCHSlotAllocation = pHeader.GetSCHslotAllocation();
    int start=0;
    int duration = 0;
    for(int i=0;i<pHeader.GetSCHSlotNum();i++)
    {
      if(hdrSCHSlotAllocation[i]==GetNode()->GetId())
      {
        start = i;
        duration++;
      }
    }
    start = start - duration + 1;
    startTxSCH = config.slotSize*start;
    m_durationSCH = config.slotSize*duration;
  }
  else if(m_type==2)
  {
    if(pHeader.GetIsLeader())
      return;
    std::map<uint16_t, uint32_t>::iterator iter;
    iter = m_queueLen.find(pHeader.GetId());
    if (iter != m_queueLen.end())
    {
      m_queueLen[iter->first] = pHeader.GetQueueLen();
    }
    else
    {
      m_queueLen.insert(std::pair<uint16_t, uint16_t>(pHeader.GetId(), pHeader.GetQueueLen()));
    }
  }
}

void
APApplication::SetupHeader(PacketHeader &hdr)
{
  hdr.SetType(1);
  hdr.SetId(GetNode()->GetId());
  hdr.SetQueueLen(txq.size());
  hdr.SetTimestamp(Simulator::Now().GetMicroSeconds());
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  hdr.SetLocLat(pos.x);
  hdr.SetLocLon(pos.y);
  if (m_type==1)
  {
    hdr.SetIsLeader(false);
  }
  else if(m_type==2)
  {
    hdr.SetIsLeader(true);
    hdr.SetCCHslotAllocation(m_CCHslotAllocation);
    hdr.SetSCHslotAllocation(m_SCHslotAllocation);
  }
}

void
APApplication::SlotAllocation()
{
  m_SCHslotAllocation.clear();
  std::map<uint16_t, uint32_t>::iterator iter;
  int count=0;
  for(iter = m_queueLen.begin();iter!=m_queueLen.end();iter++)
  {
    if(iter->second >=3)
      count=3;
    else
      count=iter->second;
    for(int i=0;i<count;i++)
      m_SCHslotAllocation.push_back(iter->first);
  }
  startTxSCH = config.slotSize*m_SCHslotAllocation.size();
  count = (txq.size()<3)?txq.size():3;
  for(int i=0;i<count;i++)
    m_SCHslotAllocation.push_back(GetNode()->GetId());
  m_durationSCH = config.slotSize*count;
}

void
APApplication::StartCCH()
{
  Ptr<Packet> pkt = Create<Packet> (200);
  PacketHeader pHeader;
  SetupHeader(pHeader);
  pkt->AddHeader(pHeader);
  Simulator::Schedule (startTxCCH, &CSMAApplication::DoSendPacket, this, pkt, CCH);
  ChangeSCH();
  Time wait = m_cchi +m_gi - MicroSeconds (Simulator::Now().GetMicroSeconds()%m_synci.GetMicroSeconds());
  if(m_type==2)
  {
    Simulator::Schedule (wait, &APApplication::SlotAllocation, this);
  }
  if(Simulator::Now().GetMicroSeconds()%(2*m_synci.GetMicroSeconds()) < m_synci.GetMicroSeconds())
  {
    if(m_durationSCH>0)
      Simulator::Schedule (startTxSCH + wait, &CSMAApplication::SendPacket, this);
  }
  else
    Simulator::Schedule (wait + MicroSeconds (rand()%1000), &CSMAApplication::SendPacket, this);
  Simulator::Schedule (m_synci, &CSMAApplication::StartCCH, this);
  std::cout<<GetNode()->GetId()<<" startTxSCH:"<<startTxSCH<<" duration:"<<m_durationSCH<<std::endl;
}

} // namespace ns3

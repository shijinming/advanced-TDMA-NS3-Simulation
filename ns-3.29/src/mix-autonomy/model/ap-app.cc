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
  startTxCCH = MicroSeconds((config.apNum-1-GetNode()->GetId())*config.slotSize);
  startTxSCH = MilliSeconds(0);
  Time temp = MicroSeconds(Simulator::Now().GetMicroSeconds()%m_synci.GetMicroSeconds());
  Time start = (temp <= (m_gi  + startTxCCH)) ? (m_gi + startTxCCH - temp) : (m_gi + startTxCCH + m_synci - temp);
  Simulator::Schedule (start, &CSMAApplication::StartCCH, this);
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
    ReceiveFromAP(pkt, app->GetVehicleType());
  }
  return true;
}

void
APApplication::ReceiveFromAP(Ptr<const Packet> pkt, uint16_t type)
{
  PacketHeader pHeader;
  pkt->PeekHeader(pHeader);
  if(type==1)
  {
    if(!pHeader.GetIsLeader())
      return;
    
  }
  else if(type==2)
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

}

} // namespace ns3

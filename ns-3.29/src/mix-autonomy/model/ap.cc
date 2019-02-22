/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ap.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/my-header.h"
#include "ns3/packet.h"
#include <vector>

namespace ns3
{
void
APFollower::ReceivePacket (Ptr<Packet> pkt, Address & srcAddr)
{ 
  PacketHeader pHeader;
  pkt->RemoveHeader(pHeader);
  std::vector <uint16_t> CCHslotAllocation = pHeader.GetCCHslotAllocation();
  std::vector <uint16_t> SCHslotAllocation = pHeader.GetSCHslotAllocation(); 
  for(uint32_t i=0; i<CCHSlotNum; i++) //查找下次发控制包的时隙
  {
    if(GetNode ()->GetId () == CCHslotAllocation[i])
      {
        CCHSendSlot = CCHSlotNum-1-i;   //可加上break跳出循环
      }
  }

  for(uint32_t i=0; i<SCHSlotNum; i++) //查找发数据包的时隙
  {
    if(GetNode ()->GetId () == CCHslotAllocation[i])
      {
        SCHSendSlot.insert(SCHSendSlot.begin(), SCHSlotNum-1-i); //将i插入到向量起始位置前
      } 
  }
}

void
APFollower::SetupHeader (PacketHeader &hdr)
{
  hdr.SetIsLeader(false);
  hdr.SetType(1);
  hdr.SetId(GetNode ()->GetId ());
  hdr.SetQueueLen(txq.size());
  hdr.SetTimestamp(Simulator::Now ().GetMicroSeconds ());
  hdr.SetLocLon(0);
  hdr.SetLocLat(0);
  hdr.SetSlotId(curSlot.id);
  hdr.SetSlotSize(curSlot.duration.GetMicroSeconds());
}

bool
APFollower::IsAPApplicationInstalled (Ptr<Node> node)
{
  uint32_t nApps = node->GetNApplications ();
  for (uint32_t idx = 0; idx < nApps; idx ++)
  {
    Ptr<Application> app = node->GetApplication (idx);
    if (dynamic_cast <APFollower*> (PeekPointer (app)) || dynamic_cast <APLeader*> (PeekPointer (app)))
    {
      return true;
    }
  }
  return false;
}

void
APLeader::ReceivePacket (Ptr<Packet> pkt, Address & srcAddr)
{
  PacketHeader pHeader;
  pkt->RemoveHeader(pHeader);
  std::map <uint16_t, uint32_t>::iterator iter;
  iter = m_queueLen.find(pHeader.GetId());
  if(iter != m_queueLen.end())
    {
      m_queueLen[iter->first] = pHeader.GetQueueLen();
    }
  else
  {
    m_queueLen.insert(std::pair<uint16_t, uint16_t> (pHeader.GetId(), pHeader.GetQueueLen()));
  }
}

void
APLeader::SetupHeader(PacketHeader &hdr)
{
  APFollower::SetupHeader(hdr);
  hdr.SetIsLeader(true);
  hdr.SetCCHslotAllocation(m_CCHslotAllocation);
  hdr.SetSCHslotAllocation(m_SCHslotAllocation);
}

void 
APLeader::SlotAllocation ()
{
  std::map <uint16_t, uint32_t>::iterator iter;
  int totalLen = 0;
  for(iter = m_queueLen.begin(); iter != m_queueLen.end(); iter++)
  {
    totalLen+=iter->second;
  }
  int index1 = 0, index2 = 0;
  for(iter = m_queueLen.begin(); iter != m_queueLen.end(); iter++)
  {
    m_CCHslotAllocation[index1] = iter->first;
    index1++;
    for(uint32_t i = 0; i < iter->second * SCHSlotNum / totalLen; i++){
      m_SCHslotAllocation[index2] = iter->first;
      index2++;
    } 
  }
}

}

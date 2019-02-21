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
  AllocationHeader pHeader;
  pkt->RemoveHeader(pHeader);
  std::vector <uint32_t> m_CCHslotAllocation = pHeader.GetCCHslotAllocation();
  std::vector <uint32_t> m_SCHslotAllocation = pHeader.GetSCHslotAllocation(); 
  for(uint32_t i=0; i<CCHSlotNum; i++) //查找下次发控制包的时隙
  {
    if(GetNode ()->GetId () == m_CCHslotAllocation[i])
      {
        CCHSendSlot = CCHSlotNum-1-i;   //可加上break跳出循环
      }
  }

  for(uint32_t i=0; i<SCHSlotNum; i++) //查找发数据包的时隙
  {
    if(GetNode ()->GetId () == m_CCHslotAllocation[i])
      {
        SCHSendSlot.insert(SCHSendSlot.begin(), SCHSlotNum-1-i); //将i插入到向量起始位置前
      } 
  }
}

void
APLeader::ReceivePacket (Ptr<Packet> pkt, Address & srcAddr)
{
  PacketHeader pHeader;
  pkt->RemoveHeader(pHeader);
  if(pHeader.GetId() == 0)
    {
      m_CCHslotAllocation.clear();
      m_SCHslotAllocation.clear();
    } 
  //控制帧配置
  m_CCHslotAllocation.insert(m_CCHslotAllocation.begin(),pHeader.GetId());
  //数据帧配置，这里Queue估计要除以速率，并上取整
  m_SCHslotAllocation.insert(m_SCHslotAllocation.begin(),pHeader.GetQueueLen(),pHeader.GetId());
}

void
APLeader::SetupHeader(AllocationHeader &hdr)
{
	Ptr<Packet> pktToSend;
  std::vector <uint32_t> m_CCHslotAllocation;
  std::vector <uint32_t> m_SCHslotAllocation;
  hdr.SetType(1);
  hdr.SetId(GetNode ()->GetId ());
  hdr.SetQueueLen(txq.size());
  hdr.SetTimestamp(Simulator::Now ().GetMicroSeconds ());
  hdr.SetLocLon(0);
  hdr.SetLocLat(0);
  hdr.SetSlotId(curSlot.id);
  hdr.SetSlotSize(curSlot.duration.GetMicroSeconds());
  hdr.SetCCHslotAllocation(m_CCHslotAllocation);
  hdr.SetSCHslotAllocation(m_SCHslotAllocation);
}

}

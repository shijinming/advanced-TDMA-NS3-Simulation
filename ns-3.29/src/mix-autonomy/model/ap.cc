/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ap.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/my-header.h"
#include "ns3/packet.h"
#include <vector>

namespace ns3
{

TypeId
APFollower::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::APFollower")
        .SetParent <TDMAApplication> ()
        .AddConstructor <APFollower> ()
    ;
    return tid;
}

APFollower::APFollower ()
{

}

APFollower::~APFollower ()
{
  
}

void
APFollower::ReceivePacket (Ptr<Packet> pkt, Address & srcAddr)
{
  InetSocketAddress addr = InetSocketAddress::ConvertFrom (srcAddr);
  Ipv4Address ipv4Addr = addr.GetIpv4 ();
  // 获取发送该数据包的节点
  Ptr<Node> node = GetNodeFromAddress (ipv4Addr);
  if (IsAPApplicationInstalled (node))
  {
    // 收到了来自内核层的数据包
    ReceivePacketFromAP (pkt);
  }
}

void
APFollower::ReceivePacketFromAP (Ptr<Packet> pkt)
{ 
  PacketHeader pHeader;
  pkt->RemoveHeader(pHeader);
  if(!pHeader.GetIsLeader())
    return;
  uint16_t *CCHslotAllocation = pHeader.GetCCHslotAllocation();
  uint16_t *SCHslotAllocation = pHeader.GetSCHslotAllocation(); 
  for(uint32_t i=0; i<curSlot.CCHSlotNum; i++) //查找下次发控制包的时隙
  {
    if(GetNode ()->GetId () == CCHslotAllocation[i])
      {
        CCHSendSlot = i;   //可加上break跳出循环
      }
  }
  SCHSendSlot.clear();
  for(uint32_t i=0; i<curSlot.SCHSlotNum; i++) //查找发数据包的时隙
  {
    if(GetNode ()->GetId () == SCHslotAllocation[i])
      {
        SCHSendSlot.push_back(i); //将i插入到向量起始位置前
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
  Vector pos = GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
  hdr.SetLocLat(pos.x);
  hdr.SetLocLon(pos.y);
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

Ptr<Node>
APFollower::GetNodeFromAddress (Ipv4Address & address)
{
    for (NodeList::Iterator n = NodeList::Begin (); 
        n != NodeList::End (); n++)
    {
        Ptr<Node> node = *n;
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
        NS_ASSERT (ipv4);
        if (ipv4->GetInterfaceForAddress (address) != -1)
        {
            return node;
        }
    }
    return NULL;
}

TypeId
APLeader::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::APLeader")
        .SetParent <TDMAApplication> ()
        .AddConstructor <APLeader> ()
    ;
    return tid;
}

APLeader::APLeader ()
{
  
}

APLeader::~APLeader ()
{
  
}

void
APLeader::ReceivePacketFromAP (Ptr<Packet> pkt)
{
  PacketHeader pHeader;
  pkt->RemoveHeader(pHeader);
  if(pHeader.GetIsLeader())
    return;
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
    for(uint32_t i = 0; i < iter->second * curSlot.SCHSlotNum / totalLen; i++){
      m_SCHslotAllocation[index2] = iter->first;
      index2++;
    } 
  }
}

}

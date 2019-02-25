/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include<cstdlib>
#include "ap-leader.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("APFollower");
NS_OBJECT_ENSURE_REGISTERED (APFollower);

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
  InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom (srcAddr);
  Ipv4Address addr = inetAddr.GetIpv4 ();
  // 获取发送该数据包的节点
  Ptr<Node> node = GetNodeFromAddress (addr);
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
        SCHSendSlot.push_back(i); //将i插入到向量最后面
      } 
  }
}

void
APFollower::SetupHeader (PacketHeader &hdr)
{
  hdr.SetIsLeader(false);
  hdr.SetType(1);
  hdr.SetId(GetNode ()->GetId ());
  hdr.SetQueueLen(rand()%100);
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

bool
APFollower::SlotAllocation ()
{
  if(curSlot.frameId == CCHSendSlot && curSlot.curFrame == CCH_apFrame)
     {
       return true;
     } 
  if(curSlot.curFrame == SCH_apFrame)
    {
      for (uint64_t i = 0; i < SCHSendSlot.size(); i++)
      {
        if(curSlot.frameId == SCHSendSlot[i]) return true;
      }
    }
  return false;
}

}

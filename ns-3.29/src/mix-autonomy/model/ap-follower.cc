/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <cstdlib>
#include "ap-leader.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("APFollower");
NS_OBJECT_ENSURE_REGISTERED(APFollower);

TypeId
APFollower::GetTypeId()
{
  static TypeId tid = TypeId("ns3::APFollower")
                          .SetParent<TDMAApplication>()
                          .AddConstructor<APFollower>();
  return tid;
}

APFollower::APFollower()
{
  leaderPacketCnt = 0;
}

APFollower::~APFollower()
{
}

void APFollower::ReceivePacket(Ptr<Packet> pkt, Address &srcAddr)
{
  InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom(srcAddr);
  Ipv4Address addr = inetAddr.GetIpv4();
  // 获取发送该数据包的节点
  Ptr<Node> node = GetNodeFromAddress(addr);
  return;
  if (IsAPApplicationInstalled(node))
  {
    // 收到了来自内核层的数据包
    bool isLeader = ReceivePacketFromAP(pkt);
    if (SCHSendSlot.size() > 0 && leaderPacketCnt == 1 && isLeader)
    {
      if (curSlot.curFrame == CCH_apFrame)
      {
        curSlot.duration = SCHSendSlot.size() * slotSize - minTxInterval;
        slotStartEvt = Simulator::Schedule((SCHSendSlot[0] + 1) * slotSize + curSlot.hdvCCHSlotNum * slotSize + minTxInterval, &APFollower::SlotStarted, this);
        //  std::cout<<GetNode()->GetId()<<" 预计在数据帧时隙"<<SCHSendSlot[0]<<"发包"<<std::endl;
      }
    }
  }
}

bool APFollower::ReceivePacketFromAP(Ptr<Packet> pkt)
{
  PacketHeader pHeader;
  pkt->RemoveHeader(pHeader);

  GetCurFrame();
  if (curSlot.curFrame == CCH_apFrame && curSlot.frameId == curSlot.apCCHSlotNum - 1)
  {
    if (pHeader.GetIsLeader())
      leaderPacketCnt++;
  }
  else
    leaderPacketCnt = 0;

  if (!pHeader.GetIsLeader())
  {
    return false;
  }
  uint16_t *CCHslotAllocation = pHeader.GetCCHslotAllocation();
  uint16_t *SCHslotAllocation = pHeader.GetSCHslotAllocation();
  for (uint32_t i = 0; i < curSlot.CCHSlotNum; i++) //查找下次发控制包的时隙
  {
    if (GetNode()->GetId() == CCHslotAllocation[i])
    {
      CCHSendSlot = i; //可加上break跳出循环
      break;
    }
  }
  SCHSendSlot.clear();
  for (uint32_t i = 0; i < curSlot.SCHSlotNum; i++) //查找发数据包的时隙
  {
    if (GetNode()->GetId() == SCHslotAllocation[i])
    {
      SCHSendSlot.push_back(i); //将i插入到向量最后面
      // std::cout<<GetNode()->GetId()<<" 收到时隙分配数组，在"<<i<<"数据时隙可发包"<<std::endl;
    }
  }
  return true;
}

void APFollower::SetupHeader(PacketHeader &hdr)
{
  hdr.SetIsLeader(false);
  hdr.SetType(1);
  hdr.SetId(GetNode()->GetId());
  hdr.SetQueueLen(txqSCH.size());
  // hdr.SetQueueLen(rand()%100);
  hdr.SetTimestamp(Simulator::Now().GetMicroSeconds());
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  hdr.SetLocLat(pos.x);
  hdr.SetLocLon(pos.y);
}

bool APFollower::IsAPApplicationInstalled(Ptr<Node> node)
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

Ptr<Node>
APFollower::GetNodeFromAddress(Ipv4Address &address)
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

struct TDMASlot
APFollower::GetNextSlotInterval(void)
{
  // LOG_UNCOND ("Get Next Slot " << GetNode ()->GetId ());
  GetCurFrame();
  curSlot.start = (curSlot.CCHSlotNum + curSlot.SCHSlotNum - 1) * slotSize + minTxInterval;
  curSlot.duration = slotSize - minTxInterval;
  return curSlot;
}

void APFollower::SendPacket(void)
{
  if (curSlot.curFrame == CCH_apFrame && isAtOwnSlot)
  {
    Ptr<Packet> pkt;
    uint32_t CpktCnt = 10;
    uint32_t SpktCnt = 10;
    for (uint32_t i = 0; i < CpktCnt; i++)
    {
      pkt = Create<Packet>(1000);
      txqCCH.push(pkt);
    }
    for (uint32_t i = 0; i < SpktCnt; i++)
    {
      pkt = Create<Packet>(1000);
      txqSCH.push(pkt);
    }
  }
}

void APFollower::SlotWillStart(void)
{
  SendPacket();
  WakeUpTxQueue();
}

} // namespace ns3

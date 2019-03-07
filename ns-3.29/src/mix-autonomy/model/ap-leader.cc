/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ap-leader.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("APLeader");
NS_OBJECT_ENSURE_REGISTERED (APLeader);

TypeId
APLeader::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::APLeader")
        .SetParent <APFollower> ()
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
APLeader::ReceivePacket (Ptr<Packet> pkt, Address & srcAddr)
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
  m_SCHslotAllocation.clear();
  if (m_CCHslotAllocation.size() == 0)
  {
      for(uint32_t i=0; i < curSlot.apCCHSlotNum; i++)
        m_CCHslotAllocation.push_back((i+1)%(config.apNum+1));
      for(uint32_t i=curSlot.apCCHSlotNum; i < curSlot.CCHSlotNum; i++)
        m_CCHslotAllocation.push_back(config.apNum + 1);   
  }
  if (m_SCHslotAllocation.size() == 0)
  {
      for(uint32_t i=0; i < curSlot.apSCHSlotNum; i++)
        m_SCHslotAllocation.push_back((i+1)%(config.apNum+1));
      for(uint32_t i=curSlot.apSCHSlotNum; i < curSlot.SCHSlotNum; i++)
        m_SCHslotAllocation.push_back(config.apNum + 1);  
  }
/*
  std::map <uint16_t, uint32_t>::iterator iter;
  iter = m_queueLen.find(GetNode () -> GetId());
  if(iter != m_queueLen.end())
    {
      m_queueLen[iter->first] = txqSCH.size();
    }
  else
  {
    m_queueLen.insert(std::pair<uint16_t, uint16_t> (GetNode ()->GetId(), txqSCH.size ()));
  }
  int totalLen = 0;
  // std::cout<<"QueueLen:"<<std::endl;
  for(iter = m_queueLen.begin(); iter != m_queueLen.end(); iter++)
  {
    totalLen+=iter->second;
    // std::cout<<iter->first<<':'<<iter->second<<' ';
  }
  std::cout<<std::endl;
  int index1 = 0;
  for(iter = m_queueLen.begin(); iter != m_queueLen.end(); iter++)
  {
    if(totalLen > 0)
    {
      for(uint32_t i = 0; i < iter->second * curSlot.apSCHSlotNum / totalLen; i++){
        m_SCHslotAllocation[index1] = iter->first;
        index1++;
      }
    }
  }
*/

  // std::cout<<"CCH slot allocation:"<<std::endl;
  // for(uint32_t i=0;i<m_CCHslotAllocation.size();i++)
  // {
  //   std::cout<<i<<':'<<m_CCHslotAllocation[i]<<' ';
  // }
  // std::cout<<std::endl;
  // std::cout<<"SCH slot allocation:"<<std::endl;
  // for(uint32_t i=0;i<m_SCHslotAllocation.size();i++)
  // {
  //   std::cout<<i<<':'<<m_SCHslotAllocation[i]<<' ';
  // }
  // std::cout<<std::endl;
  //查找leader给自己分配的数据帧发包时隙
  for(uint32_t i=0; i<curSlot.apSCHSlotNum; i++) //查找发数据包的时隙
  {
    if(GetNode ()->GetId () == m_SCHslotAllocation[i])
    {
      mySendSlot.push_back(i); //将i插入到向量最后面
    } 
  }
}

void
APLeader::SlotDidEnd (void)
{
  std::cout<<"leader SCH size:"<<mySendSlot.size()<<std::endl;
  if(mySendSlot.size()>0)
    {
      std::cout<<"leader SSH slot:"<<mySendSlot[0]<<std::endl;
      curSlot.duration = mySendSlot.size()* slotSize - minTxInterval;
      std::cout<<GetNode()->GetId()<<':'<<"Simulator::Schedule ("
       <<mySendSlot[0] * slotSize + curSlot.hdvCCHSlotNum * slotSize + minTxInterval
       <<", &APLeader::SlotStarted, this)"<<std::endl;
      slotStartEvt = Simulator::Schedule (mySendSlot[0] * slotSize + curSlot.hdvCCHSlotNum * slotSize + minTxInterval, &APLeader::SlotStarted, this);
    }
    mySendSlot.clear();
}

}

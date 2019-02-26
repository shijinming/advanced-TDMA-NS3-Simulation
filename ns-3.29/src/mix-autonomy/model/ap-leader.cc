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
  if (m_CCHslotAllocation.size() == 0)
  {
      for(uint32_t i=0; i < curSlot.CCHSlotNum; i++)
        m_CCHslotAllocation.push_back(i);
  }
  if (m_SCHslotAllocation.size() == 0)
  {
      for(uint32_t i=0; i < curSlot.SCHSlotNum; i++)
        m_SCHslotAllocation.push_back(0);
  }
  std::map <uint16_t, uint32_t>::iterator iter;
  iter = m_queueLen.find(GetNode () -> GetId());
  if(iter != m_queueLen.end())
    {
      m_queueLen[iter->first] = txq.size();
    }
  else
  {
    m_queueLen.insert(std::pair<uint16_t, uint16_t> (GetNode ()->GetId(), txq.size ()));
  }
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
    if(totalLen > 0)
    {
      for(uint32_t i = 0; i < iter->second * curSlot.SCHSlotNum / totalLen; i++){
        m_SCHslotAllocation[index2] = iter->first;
        index2++;
      }
    } 
  }
  std::cout<<"CCH slot allocation:"<<std::endl;
  for(uint32_t i=0;i<m_CCHslotAllocation.size();i++)
  {
    std::cout<<i<<':'<<m_CCHslotAllocation[i]<<' ';
  }
  std::cout<<std::endl;
  std::cout<<"SCH slot allocation:"<<std::endl;
  for(uint32_t i=0;i<m_CCHslotAllocation.size();i++)
  {
    std::cout<<i<<':'<<m_SCHslotAllocation[i]<<' ';
  }
  std::cout<<std::endl;
  //查找leader给自己分配的数据帧发包时隙
  for(uint32_t i=0; i<curSlot.SCHSlotNum; i++) //查找发数据包的时隙
  {
    if(GetNode ()->GetId () == m_SCHslotAllocation[i])
      {
        mySendSlot.push_back(i); //将i插入到向量最后面
      } 
  }
  if(!mySendSlot.size())
    {
     curSlot.duration = mySendSlot.size()* slotSize - minTxInterval;
     slotStartEvt = Simulator::Schedule (mySendSlot[0] * slotSize + minTxInterval, &APLeader::SlotStarted, this);
    }
}

}
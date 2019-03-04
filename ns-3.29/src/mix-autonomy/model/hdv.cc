/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hdv.h"
#include "ap-leader.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("HumanApplication");
NS_OBJECT_ENSURE_REGISTERED (HumanApplication);

TypeId
HumanApplication::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::HumanApplication")
        .SetParent <TDMAApplication> ()
        .AddConstructor <HumanApplication> ()
    ;
    return tid;
}

HumanApplication::HumanApplication ()
{
    NS_LOG_FUNCTION (this);
    m_status = Outter;
}

HumanApplication::~HumanApplication ()
{
    NS_LOG_FUNCTION (this);
}

int
HumanApplication::GetStatus ()
{
  if(m_status == Middle)
    return 1;
  else
    return 2;
}

void
HumanApplication::AddToMiddle ()
{
  m_status = Middle;
  std::cout<<GetNode()->GetId()<<" add to middle."<<std::endl;
  slotEndEvt.Cancel();
  txEvent.Cancel ();
  Time t1,t2,t3;
  t1 = Simulator::Now() - minTxInterval;
  t2 = (curSlot.CCHSlotNum + curSlot.SCHSlotNum) * slotSize;
  t3 = MilliSeconds(t1.GetMilliSeconds()%t2.GetMilliSeconds());
  if(t3.GetMilliSeconds() <= uint32_t (curSlot.apCCHSlotNum) * slotSize.GetMilliSeconds())
  {
    curSlot.start = curSlot.apCCHSlotNum * slotSize - t3;
  }
  else curSlot.start = t2 + curSlot.apCCHSlotNum * slotSize - t3;
  curSlot.duration = slotSize * curSlot.hdvCCHSlotNum - minTxInterval;  
  slotStartEvt = Simulator::Schedule (curSlot.start, &HumanApplication::SlotStarted, this);
  isAtOwnSlot = false;
} 

void 
HumanApplication::QuitFromMiddle ()
{
    m_status = Outter;
    std::cout<<GetNode()->GetId()<<" quit from middle."<<std::endl;
}

void 
HumanApplication::ReceivePacket (Ptr<Packet> pkt, Address & srcAddr)
{
  InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom (srcAddr);
  Ipv4Address addr = inetAddr.GetIpv4 ();
    // 获取发送该数据包的节点
    Ptr<Node> node = GetNodeFromAddress (addr);
    if (IsAPApplicationInstalled (node))
    {
        // 收到了来自内核层的数据包
        if(m_status == Outter)
          AddToMiddle ();
        receiveAPId = curSlot.id;
    }
    else if (curSlot.id-receiveAPId > 2 * (curSlot.CCHSlotNum + curSlot.SCHSlotNum)) //一个总帧内未收到内核层的包
    {
      if(m_status == Middle)
        QuitFromMiddle();
    }
}


Ptr<Node>
HumanApplication::GetNodeFromAddress (Ipv4Address & address)
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
HumanApplication::IsAPApplicationInstalled (Ptr<Node> node)
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

struct TDMASlot 
HumanApplication::GetNextSlotInterval (void)
{
  //LOG_UNCOND ("Get Next Slot " << GetNode ()->GetId ());
  SetCurSlot();
  if(GetStatus () == Middle)
  {
     if(curSlot.curFrame == CCH_apFrame || curSlot.curFrame == SCH_hdvFrame)
     {
      curSlot.start = curSlot.apCCHSlotNum *slotSize + minTxInterval;
      curSlot.duration = slotSize * curSlot.hdvCCHSlotNum - minTxInterval; 
     }
     else
     {
      curSlot.start = curSlot.apSCHSlotNum *slotSize + minTxInterval;
      curSlot.duration = slotSize * curSlot.hdvSCHSlotNum - minTxInterval; 
     }
  }
  else 
  {
    curSlot.start = minTxInterval;
    curSlot.duration = Seconds(config.simTime);
  }
  return curSlot;
}

void
HumanApplication::SendPacket (void)
{
  if (curSlot.curFrame == CCH_hdvFrame && isAtOwnSlot)
  {
    Ptr<Packet> pkt;
    uint32_t CpktCnt = 5;
    uint32_t SpktCnt = rand()%20;
    for(uint32_t i = 0; i < CpktCnt; i++)
    {
      pkt = Create<Packet> (0);
      txqCCH.push(pkt);
    }
    for(uint32_t i = 0; i < SpktCnt; i++)
    {
      pkt = Create<Packet> (0);
      txqSCH.push(pkt);
    }
  }
}

void 
HumanApplication::SlotWillStart (void)
{
  SendPacket ();
}

}
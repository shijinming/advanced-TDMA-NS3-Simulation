#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include"ns3/wave-module.h"
#include "ns3/wifi-net-device.h"
#include "tdma-app.h"
#include <fstream>


namespace ns3 {

TypeId
TDMAApplication::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TDMAApplication")
    .SetParent<Application> ()
    .AddAttribute ("DataRate", "Data rate",
      DataRateValue (DataRate ("1Mb/s")),
      MakeDataRateAccessor (&TDMAApplication::dataRate),
      MakeDataRateChecker ())
    .AddAttribute ("EnableMockTraffic", "Whether to enable mock traffic",
      BooleanValue (false),
      MakeBooleanAccessor (&TDMAApplication::enableMockTraffic),
      MakeBooleanChecker ())
    .AddAttribute ("MockPacketSize", "Size of mock packets (in bytes)",
      UintegerValue (200),
      MakeUintegerAccessor (&TDMAApplication::mockPktSize),
      MakeUintegerChecker<uint32_t> (0))
    .AddAttribute ("MinTxInterval", "Minimal Tx Interval",
      TimeValue (MicroSeconds (100)),
      MakeTimeAccessor (&TDMAApplication::minTxInterval),
      MakeTimeChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent", 
      MakeTraceSourceAccessor (&TDMAApplication::txTrace),
      "ns3::Packet::AddressTraceCallback")
    .AddTraceSource ("Rx", "A new packet is received by sink",
      MakeTraceSourceAccessor (&TDMAApplication::rxTrace),
      "ns3::ThreeLayerHelper::TracedCallback");
  return tid;
}

TDMAApplication::TDMAApplication ()
  : isAtOwnSlot (false),
    enableMockTraffic (true),
    config (SimulationConfig::Default ())
{
}

TDMAApplication::~TDMAApplication () 
{
}

void
TDMAApplication::SetStartTime (Time start) 
{
  LOG_UNCOND ("不要手动设置SetStartTime，重载GetInitalSlot函数");
  exit (1);
}

void
TDMAApplication::SetStopTime (Time stop) {
  LOG_UNCOND ("WARN: 不要手动设置SetStopTime，系统会自动设置为仿真结束");
  Application::SetStopTime (stop);
}

void 
TDMAApplication::DoInitialize (void) 
{
  std::ifstream f(config.startTimeFile);
  if (!f.is_open())
    std::cout<<"start time file is not open!"<<std::endl;
  float t=0;
  for (uint32_t i=0;i<=GetNode()->GetId();i++)
    f>>t;
  curSlot = GetInitalSlot(Seconds(t));
  m_startTime = curSlot.start;
  m_stopTime = Seconds (config.simTime);

  Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetNode ()->GetDevice (0));
  Ptr<WifiPhy> phy = device->GetPhy ();
  phy->TraceConnectWithoutContext("PhyTxBegin", MakeCallback(&TDMAApplication::WifiPhyTxBeginTrace, this));
  // phy->TraceConnectWithoutContext("PhyRxBegin", MakeCallback(&TDMAApplication::WifiPhyRxBeginTrace, this));

  // 在基类函数中，会根据m_startTime设置一个定时器来调用StartApplication
  Application::DoInitialize ();
}

void
TDMAApplication::DoDispose (void) 
{
  socket = NULL;
  sink = NULL;
}

void 
TDMAApplication::StartApplication (void) 
{
  CreateSocket ();
  // 第一个时隙开始
  SlotStarted ();
  OutputPosition ();
  std::cout<<GetNode()->GetId()<<" starts at "<<Simulator::Now()<<std::endl;
}

void 
TDMAApplication::StopApplication (void) 
{
  CancelAllEvents ();
}

void
TDMAApplication::CancelAllEvents (void)
{
  txEvent.Cancel ();
  slotEndEvt.Cancel ();
  slotStartEvt.Cancel ();
  position.Cancel ();
}

void
TDMAApplication::SlotEnded (void) 
{
  // LOG_UNCOND ("Slot of " << GetNode ()->GetId () << " ended at " << Simulator::Now ().GetMicroSeconds ());
  if (!isAtOwnSlot) {
    LOG_UNCOND ("Fatal Error[1]: 时隙调度错误");
    exit (1);
  }
  // txEvent.Cancel ();
  GetNextSlotInterval ();

  if(curSlot.curFrame == Frame::CCH_apFrame || curSlot.curFrame == Frame::CCH_hdvFrame)
    std::cout<<"CCH:";
  else
    std::cout<<"SCH:";
  std::cout<<GetNode()->GetId()<<" SlotEnded "<<Simulator::Now().GetMilliSeconds()<<std::endl;

  if(IsAPApplicationInstalled(GetNode()))
  {
    if(curSlot.curFrame == CCH_apFrame || curSlot.curFrame == CCH_hdvFrame)
    {
      slotStartEvt = Simulator::Schedule (curSlot.start, &TDMAApplication::SlotStarted, this);     
    }
  }
  else
  {
    slotStartEvt = Simulator::Schedule (curSlot.start, &TDMAApplication::SlotStarted, this);
  }
  
  isAtOwnSlot = false;
  if(curSlot.curFrame == CCH_apFrame || curSlot.curFrame == CCH_hdvFrame) 
  {
    SlotDidEnd ();
  }
}

void
TDMAApplication::SlotStarted (void) 
{
  // LOG_UNCOND ("Slot of " << GetNode ()->GetId () << " started at " << Simulator::Now ().GetMicroSeconds ());
  GetCurFrame();
  if(curSlot.curFrame == Frame::CCH_apFrame || curSlot.curFrame == Frame::CCH_hdvFrame)
    std::cout<<"CCH:";
  else
    std::cout<<"SCH:";
  std::cout<<GetNode()->GetId()<<" SlotStarted "<<Simulator::Now().GetMilliSeconds()<<std::endl;

  if (isAtOwnSlot) {
    // 已经开始了的时隙重复启动
    LOG_UNCOND ("Fatal Error[0]: 时隙调度错误");
    exit (1);
  }
  slotEndEvt = Simulator::Schedule (curSlot.duration, &TDMAApplication::SlotEnded, this);

  if(curSlot.curFrame == SCH_apFrame)  
    curSlot.duration = slotSize - minTxInterval;
  slotCnt += 1;
  isAtOwnSlot = true;
  if(curSlot.curFrame == Frame::CCH_apFrame)
    SlotAllocation();
  SlotWillStart ();
  // std::cout<<GetNode ()->GetId ()<<" CCH queue:"<<txqCCH.size()<<" SSH queue:"<<txqSCH.size()<<std::endl;
  WakeUpTxQueue ();
}

void 
TDMAApplication::CreateSocket (void)
{
  auto broadcastAddr = InetSocketAddress (Ipv4Address ("255.255.255.255"), config.socketPort);
  socketTid = UdpSocketFactory::GetTypeId ();
  if (!socket) 
  {
    socket = Socket::CreateSocket (GetNode (), socketTid);
    if (socket->Bind ())
    {
      LOG_UNCOND ("Fatal Error: Fail to bind socket");
      exit (1);
    }
    socket->Connect (broadcastAddr);
    socket->SetAllowBroadcast (true);
    socket->ShutdownRecv ();
  }
  if (!sink)
    {
      sink = Socket::CreateSocket (GetNode (), socketTid);
      if (sink->Bind (Address (InetSocketAddress (Ipv4Address::GetAny (), config.socketPort))))
        {
          LOG_UNCOND ("Fatal Error: Fail to bind socket");
          exit (1);
        }
      sink->Listen ();
      sink->ShutdownSend ();
      sink->SetRecvCallback (MakeCallback (&TDMAApplication::OnReceivePacket, this));
    }
}

void
TDMAApplication::OnReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> pkt;
  Address srcAddr;
  while ((pkt = sink->RecvFrom (srcAddr)))
    {
      InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom (srcAddr);
      Address addr = inetAddr.GetIpv4 ();
      ReceivePacket (pkt, srcAddr);
      rxTrace (pkt, this, addr);
    }
}

void
TDMAApplication::DoSendPacket (Ptr<Packet> pkt)
{
  PacketHeader pktHdr;
  SetupHeader (pktHdr);
  
  pkt->AddHeader(pktHdr);
  socket->Send (pkt);
//   std::cout<<GetNode()->GetId()<<" socket send "<<pkt->GetUid()<<std::endl;
  Ptr<Ipv4> ipv4 = GetNode ()->GetObject<Ipv4> ();
  txTrace (pkt, ipv4->GetAddress (1, 0).GetLocal ());
}

void
TDMAApplication::WakeUpTxQueue ()
{
  if (!isAtOwnSlot) return; 
  Ptr<Packet> pktToSend = NULL;
  GetCurFrame();
  if (curSlot.curFrame == Frame::CCH_apFrame || curSlot.curFrame == Frame::CCH_hdvFrame)
  {
    if(!txqCCH.empty ())
    {
      pktToSend = txqCCH.front ();
      txqCCH.pop ();
    }
    else if (enableMockTraffic)
    {
      pktToSend = Create<Packet> (mockPktSize);
      WillSendMockPacket (pktToSend);
    }
  }
  else
  {
    if(!txqSCH.empty ())
    {
      pktToSend = txqSCH.front ();
      txqSCH.pop ();
    }
    else if (enableMockTraffic)
    {
      pktToSend = Create<Packet> (mockPktSize);
      WillSendMockPacket (pktToSend);
    }
  }
  Time nextTxTime = minTxInterval;
  if (pktToSend != NULL)
    {
      DoSendPacket (pktToSend);
      // nextTxTime = Max(nextTxTime, Seconds (
      //   (pktToSend->GetSize () * 8) / static_cast<double>(dataRate.GetBitRate ())
      // ));
    }
  
  // Schedule Next Tx
  // txEvent = Simulator::Schedule (nextTxTime, &TDMAApplication::WakeUpTxQueue, this);
}

 void
 TDMAApplication::SwitchToNextChannel (uint32_t curChannelNumber, uint32_t nextChannelNumber)
 {
  Ptr<WaveNetDevice> device = GetNode ()->GetObject<WaveNetDevice> ();
  Ptr<WifiPhy> phy = device->GetPhy (0);
  if (phy->GetChannelNumber () == nextChannelNumber)
    {
      return;
    }
  Ptr<OcbWifiMac> curMacEntity = device->GetMac (curChannelNumber);
  Ptr<OcbWifiMac> nextMacEntity = device->GetMac (nextChannelNumber);
  curMacEntity->Suspend ();
  curMacEntity->ResetWifiPhy ();
  phy->SetChannelNumber (nextChannelNumber);
  Time switchTime = phy->GetChannelSwitchDelay ();
  nextMacEntity->MakeVirtualBusy (switchTime);
  nextMacEntity->SetWifiPhy (phy);
  nextMacEntity->Resume ();
}

void
TDMAApplication::PeriodicSwitch (struct TDMASlot curSlot)
{
  if (curSlot.curFrame == Frame::CCH_hdvFrame && curSlot.frameId == curSlot.hdvCCHSlotNum -1)
    {
      SwitchToNextChannel (tdma_CCH, tdma_SCH1);
    }
  if (curSlot.curFrame == Frame::SCH_hdvFrame && curSlot.frameId == curSlot.hdvSCHSlotNum -1)
    {
      SwitchToNextChannel (tdma_SCH1, tdma_CCH);
    }
}

void
TDMAApplication:: SetCurSlot(void)
{
  // curSlot.CCHSlotNum = 2*(config.apNum+1);
  // curSlot.SCHSlotNum = 2*(config.apNum+1); 
  // curSlot.apCCHSlotNum = config.apNum+1;  
  // curSlot.apSCHSlotNum = config.apNum+1;  
  // curSlot.hdvCCHSlotNum = config.apNum+1; 
  // curSlot.hdvSCHSlotNum = config.apNum+1; 
  
  curSlot.CCHSlotNum = 1*(config.apNum+1);
  curSlot.SCHSlotNum = 1*(config.apNum+1); 
  curSlot.apCCHSlotNum = config.apNum+1;  
  curSlot.apSCHSlotNum = config.apNum+1;  
  curSlot.hdvCCHSlotNum = 0; 
  curSlot.hdvSCHSlotNum = 0;   
}

void
TDMAApplication::GetCurFrame (void)
{
  slotId = Simulator::Now ().GetMilliSeconds ()/slotSize.GetMilliSeconds();
  curSlot.id = slotId;
  uint64_t slot = slotId%(curSlot.CCHSlotNum + curSlot.SCHSlotNum);
  if(slot == 0) 
  {
    curSlot.frameNum = curSlot.frameNum +1;
  }
  if(slot < curSlot.apCCHSlotNum )
  {
    curSlot.curFrame = CCH_apFrame;
    curSlot.frameId = slot;
  }
  else if(slot < (curSlot.apCCHSlotNum + curSlot.hdvCCHSlotNum))
  {
    curSlot.curFrame = CCH_hdvFrame;
    curSlot.frameId = slot-curSlot.apCCHSlotNum;
  }
  else if(slot < (curSlot.apCCHSlotNum+curSlot.hdvCCHSlotNum + curSlot.SCHSlotNum))
  {
    curSlot.curFrame = SCH_apFrame;
    curSlot.frameId = slot - curSlot.apCCHSlotNum - curSlot.hdvCCHSlotNum;
  }
  else 
  {
    curSlot.curFrame = SCH_hdvFrame;
    curSlot.frameId = slot-curSlot.apCCHSlotNum-curSlot.hdvCCHSlotNum-curSlot.apSCHSlotNum;
  }
}

struct TDMASlot
TDMAApplication:: GetInitalSlot (Time start)
{
  // LOG_UNCOND ("Get Initial Slot " << GetNode ()->GetId ());
  SetCurSlot();
  Time willStart = MilliSeconds (720 - Simulator::Now().GetMilliSeconds()%720);
  if(GetNode ()->GetId () == 0) 
  {
    curSlot.duration =  slotSize - minTxInterval;
    curSlot.start = willStart + slotSize * config.apNum + minTxInterval;

  }
  else if(GetNode ()->GetId () < config.apNum) 
  {
    curSlot.duration =  slotSize - minTxInterval;
    curSlot.start = willStart + slotSize * (config.apNum-1-GetNode ()->GetId ()) + minTxInterval;
  }
  else 
  {
    // curSlot.start = slotSize * (config.apNum + 1) + minTxInterval;
    curSlot.start = start;
    curSlot.duration = Seconds(config.simTime);
  }
  return curSlot;
}

void
TDMAApplication::OutputPosition (void)
{
  std::cout<<GetNode ()->GetId ()<<" position: "<<GetNode ()->GetObject<ConstantVelocityMobilityModel>()->GetPosition().x
      <<' '<<Simulator::Now().GetMilliSeconds()<<std::endl;
  position = Simulator::Schedule (MilliSeconds(20), &TDMAApplication::OutputPosition, this);
}

void 
TDMAApplication::WifiPhyTxBeginTrace (Ptr<const Packet> p)
{
  WakeUpTxQueue ();
  // WifiMacHeader hdr;
  // p->PeekHeader(hdr);
  // std::cout<<hdr.GetAddr2()<<','<<p->GetUid ()<<','<<Simulator::Now().GetMicroSeconds()<<std::endl;
}

void 
TDMAApplication::WifiPhyRxBeginTrace (Ptr<const Packet> p)
{
  // WifiMacHeader hdr;
  // p->PeekHeader(hdr);
  //   std::cout<<"1,"<<','<<hdr.GetAddr2()<<','<<p->GetUid ()<<','<<Simulator::Now().GetMicroSeconds()<<std::endl;
}

}

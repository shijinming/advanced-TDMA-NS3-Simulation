#include "ns3/internet-module.h"
#include "tdma-app.h"


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
      BooleanValue (true),
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
  curSlot = GetInitalSlot ();
  m_startTime = curSlot.start;
  m_stopTime = Seconds (config.simTime);
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
  // 第一个时隙开始
  SlotStarted ();
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
}

void
TDMAApplication::SlotEnded (void) 
{
  LOG_UNCOND ("Slot of " << GetNode ()->GetId () << " ended at " << Simulator::Now ().GetMicroSeconds ());
  if (!isAtOwnSlot) {
    LOG_UNCOND ("Fatal Error[1]: 时隙调度错误");
    exit (1);
  }
  txEvent.Cancel ();
  curSlot = GetNextSlotInterval ();
  slotStartEvt = Simulator::Schedule (curSlot.start, &TDMAApplication::SlotStarted, this);
  isAtOwnSlot = false;
  SlotDidEnd ();
}

void
TDMAApplication::SlotStarted (void) 
{
  LOG_UNCOND ("Slot of " << GetNode ()->GetId () << " started at " << Simulator::Now ().GetMicroSeconds ());
  if (isAtOwnSlot) {
    // 已经开始了的时隙重复启动
    LOG_UNCOND ("Fatal Error[0]: 时隙调度错误");
    exit (1);
  }
  slotEndEvt = Simulator::Schedule (curSlot.duration, &TDMAApplication::SlotEnded, this);
  slotCnt += 1;
  isAtOwnSlot = true;
  SlotWillStart ();
  CreateSocket();
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
      if (sink->Bind ())
        {
          LOG_UNCOND ("Fatal Error: Fail to bind socket");
          exit (1);
        }
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
      ReceivePacket (pkt, addr);
      PacketHeader pHeader;
      pkt->RemoveHeader(pHeader);
      std::cout<<"Received a packet."<<std::endl;
      pHeader.Print(std::cout);
      rxTrace (pkt, this, addr);
    }
}

void
TDMAApplication::SendPacket (Ptr<Packet> pkt)
{
  txq.push (pkt);
}

void
TDMAApplication::DoSendPacket (Ptr<Packet> pkt)
{
  SetHeader();
  pkt->AddHeader(pktHeader);
  socket->Send (pkt);
  Ptr<Ipv4> ipv4 = GetNode ()->GetObject<Ipv4> ();
  txTrace (pkt, ipv4->GetAddress (1, 0).GetLocal ());
}

void
TDMAApplication::WakeUpTxQueue ()
{
  if (!isAtOwnSlot) return; 
  Ptr<Packet> pktToSend = NULL;
  if (!txq.empty ())
    {
      std::cout<<"Send normal packet."<<std::endl;
      pktToSend = txq.front ();
      txq.pop ();
    }
  else if (enableMockTraffic)
    {
      pktToSend = Create<Packet> (mockPktSize);
      WillSendMockPacket (pktToSend);
    }
  Time nextTxTime = minTxInterval;
  if (pktToSend != NULL)
    {
      DoSendPacket (pktToSend);
      nextTxTime = Max(nextTxTime, Seconds (
        (pktToSend->GetSize () * 8) / static_cast<double>(dataRate.GetBitRate ())
      ));
    }
  
  // Schedule Next Tx
  txEvent = Simulator::Schedule (nextTxTime, &TDMAApplication::WakeUpTxQueue, this);
}

void
TDMAApplication::SetHeader()
{
  pktHeader.setIsAP(1);
  pktHeader.setIsMiddle(0);
  pktHeader.setId(GetNode ()->GetId ());
  pktHeader.setQueueLen(txq.size());
  pktHeader.setTimestamp(Simulator::Now ().GetMicroSeconds ());
  pktHeader.setLocLon(0);
  pktHeader.setLocLat(0);
  pktHeader.setSlotId(curSlot.id);
  pktHeader.setSlotSize(curSlot.duration.GetMicroSeconds());
}

}
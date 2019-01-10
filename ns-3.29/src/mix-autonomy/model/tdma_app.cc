#include "tdma_app.h"


namespace ns3 {

TypeId
TDMAApplication::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TDMAApplication")
    .SetParent<Application> ();
  return tid;
}

TDMAApplication::TDMAApplication ()
  : config (SimulationConfig::Default ()) 
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
  curSlot = GetNextSlotInterval ();
  slotStartEvt = Simulator::Schedule (curSlot.start, &TDMAApplication::SlotStarted, this);
  isAtOwnSlot = false;
  SlotDidEnd ();
}

void
TDMAApplication::SlotStarted (void) 
{
  slotEndEvt = Simulator::Schedule (curSlot.duration, &TDMAApplication::SlotEnded, this);
  slotCnt += 1;
  isAtOwnSlot = true;
  SlotDidStart ();
}

void 
TDMAApplication::CreateSocket (void)
{
  InetSocketAddress broadcastAddr = InetSocketAddress (Ipv4Address ("255.255.255.255"), config.socketPort);
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
      if (!sink->Bind ())
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
      m_rxTrace (pkt, this, addr);
    }
}

}
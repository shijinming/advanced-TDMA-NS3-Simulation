/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "human-app.h"
#include "ns3/internet-module.h"
#include "ns3/wave-module.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/wifi-module.h"
#include "ns3/three-layer-helper.h"
#include "ns3/three-layer-packet.h"
#include "ns3/three-layer-event.h"

#include "core-layer.h"
#include "ns3/mix-autonomy-common.h"
#include "ns3/middle-layer.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("HumanApplication");
NS_OBJECT_ENSURE_REGISTERED (HumanApplication);

TypeId
HumanApplication::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::HumanApplication")
        .SetParent <Application> ()
        .AddConstructor <HumanApplication> ()
        .AddAttribute ("DataRate", "Data rate",
            DataRateValue (DataRate ("500kb/s")),
            MakeDataRateAccessor (&HumanApplication::m_dataRate),
            MakeDataRateChecker ())
        .AddAttribute ("PacketSize", "Size of the packet, in bytes",
            UintegerValue (200),
            MakeUintegerAccessor (&HumanApplication::m_pktSize),
            MakeUintegerChecker<uint32_t> (0))
        .AddAttribute ("BeaconInterval", "Interval of beacons, in ms",
            UintegerValue (10),
            MakeUintegerAccessor (&HumanApplication::m_beaconInterval),
            MakeUintegerChecker<uint32_t> (0))
        .AddAttribute ("StartDelay", "Start transmision after this delay, in ms",
            UintegerValue (1000),
            MakeUintegerAccessor (&HumanApplication::m_startDelay),
            MakeUintegerChecker<uint32_t> (0))
        .AddAttribute ("CheckPeriod", "Period of status check, in ms",
            UintegerValue (100),
            MakeUintegerAccessor (&HumanApplication::m_checkPeriod),
            MakeUintegerChecker<uint32_t> (0))
        .AddAttribute ("OutInterval", "Set as outter layer of no core packet received within this time, in ms",
            UintegerValue (1000),
            MakeUintegerAccessor (&HumanApplication::m_setOutInterval),
            MakeUintegerChecker<uint32_t> (0))
        .AddAttribute ("Protocol", "The type of protocol to use, This should be "
            "a subclass of ns3::SocketFactory",
            TypeIdValue (UdpSocketFactory::GetTypeId ()),
            MakeTypeIdAccessor (&HumanApplication::m_tid),
            MakeTypeIdChecker ())
        .AddAttribute ("Remote", "The Address of the destination",
            AddressValue (),
            MakeAddressAccessor (&HumanApplication::m_peer),
            MakeAddressChecker ())
        .AddAttribute ("EnableTDMA", "Enable TDMA at initial state",
            BooleanValue (false), 
            MakeBooleanAccessor (&HumanApplication::m_tdmaEnabled),
            MakeBooleanChecker ())
        .AddTraceSource ("Tx", "A new packet is created and is sent",
            MakeTraceSourceAccessor (&HumanApplication::m_txTrace),
            "ns3::Packet::AddressTraceCallback")
        .AddTraceSource ("Rx", "A packet is received by sink",
            MakeTraceSourceAccessor (&HumanApplication::m_rxTrace),
            "ns3::ThreeLayerHelper::TracedCallback")
    ;
    return tid;
}

HumanApplication::HumanApplication ()
  : m_verbose (false),
    m_status (Status::Outter),
    m_socket (0),
    m_sinkSocket (0),
    m_stream (111),  //随意设的值
    config (MyGlobalConfig::Default ())
{
}

HumanApplication::~HumanApplication ()
{
    NS_LOG_FUNCTION (this);
}

void
HumanApplication::DoDispose ()
{
    NS_LOG_FUNCTION (this);
    m_socket = 0;
    m_sinkSocket = 0;
}

void 
HumanApplication::StartApplication ()
{
    NS_LOG_FUNCTION (this);
    TL_EVENT_LOG ("Human Application of Node " << GetNode ()->GetId () << " start");

    if (config.nHumanSlots == 0) {
        // 确保可以发送
        m_tdmaEnabled = true;
    }
    TryCreateSockets ();
    
    ScheduleFirstTx ();
    PeriodicCheckStatus ();
}

void
HumanApplication::TryCreateSockets () 
{
    if (!m_socket)
    {
        CreateSendSocket ();
    }
    if (!m_sinkSocket)
    {
        CreateRecvSink ();
    }
}

void 
HumanApplication::StopApplication ()
{
    NS_LOG_FUNCTION (this);
    TL_EVENT_LOG ("Human Application of Node " << GetNode ()->GetId () << " stop");
    CancelAllEvents ();
    if (m_socket) m_socket->Close ();
    if (m_sinkSocket) m_sinkSocket->Close ();
}

void
HumanApplication::ReceivePacket (Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address srcAddr;  //src是source的缩写
    Address localAddr;
    // 获取接收数据包的本地地址
    socket->GetSockName (localAddr);
    // 接收数据包，srcAddr中会记录这个包的发送者
    while ((packet = socket->RecvFrom (srcAddr)))
    {
        InetSocketAddress addr = InetSocketAddress::ConvertFrom (srcAddr);
        Ipv4Address ipv4Addr = addr.GetIpv4 ();
        // 获取发送该数据包的节点
        Ptr<Node> node = ThreeLayerHelper::GetNodeFromAddress (ipv4Addr);
        MY_ASSERT (node != NULL, "Invalid Address: " << ipv4Addr);
        if (CoreLayerApplication::IsCoreLayerApplicationInstalled (node))
        {
            // 收到了来自内核层的数据包
            RecvFromCoreLayer (packet, node);
        }
        else
        {
            // 收到了来自其他HDV的数据包
            RecvFromOtherHuman (packet, node);
        }
        m_rxTrace (packet, this, ipv4Addr);
    }
}

int 
HumanApplication::GetHumanStatus (Ptr<Node> node)
{
    uint32_t nApps = node->GetNApplications ();
    for (uint32_t idx = 0; idx < nApps; idx ++ )
    {
        Ptr<Application> app = node->GetApplication (idx);
        Ptr<HumanApplication> human;
        if ((human = dynamic_cast<HumanApplication*> (PeekPointer (app))))
        {
            return human->GetStatus ();
        }
    }
    return -1;
}

HumanApplication::Status
HumanApplication::GetStatus ()
{
    return m_status;
}

//以下两函数是不是用来判断此车的类别，如果接收到核心层的包，则将其标记为中间层车辆；如果接收到hdv，需要判断hdv类别和加一后是否仍在条数限制内，如果满足条件，也可标记为中间层
void 
HumanApplication::RecvFromCoreLayer (Ptr<Packet> packet, Ptr<Node> node)
{
    if (config.nHumanSlots == 0) {
        // 中间层时隙为0时，中间层机制已经停用，不需要再检测层状态
        return;
    }
    m_lastTimeFromCore = Simulator::Now ();
    if (m_status == Status::Outter)
    {
        // Become Middle Layer
        m_status = Status::Middle;
        // 直接收到内核层的数据包，跳跃数为1
        m_middleHops = 1;
        // 注册到的MiddleLayerGroup
        MAMiddleLayerGroup& middle = MAMiddleLayerGroup::Default ();  //MA是什么的缩写？
        middle.Register (this);
        TL_EVENT_LOG ("Node " << GetNode ()->GetId () << " become middle");
    }
}

void 
HumanApplication::RecvFromOtherHuman (Ptr<Packet> packet, Ptr<Node> node)
{
    if (config.nHumanSlots == 0) {
        // 中间层时隙为0时，中间层机制已经停用，不需要再检测层状态
        return;
    }
    uint32_t nApps = node->GetNApplications ();
    for (uint32_t idx = 0; idx < nApps; idx ++) {
        Ptr<Application> app = node->GetApplication (idx);
        Ptr<HumanApplication> human = dynamic_cast<HumanApplication*> (PeekPointer (app));
        MY_ASSERT (human != NULL, "HumanApplication not installed correctly");
        MY_ASSERT (PeekPointer (app) != this && node->GetId () != GetNode ()->GetId (), 
            "Receving from packet from node self");
        // 忽略掉来自外围层的数据
        if (human->m_status == Status::Outter) {
            return;
        }
        // 超过中间层条约束限制，返回
        if (human->m_middleHops >= MyGlobalConfig::Default ().middleLayerHopsLimit) {
            return;
        }
        // 来到这里说明：
        // 1. 发送节点是中间层节点
        // 2. 尚未达到中间层跳跃数量限制
        // 成为中间层节点！注册过程参见RecvFromCoreLayer的过程
        m_lastTimeFromCore = Simulator::Now ();
        if (m_status != Status::Middle) {
            m_status = Status::Middle;
            m_middleHops = human->m_middleHops + 1;
            MAMiddleLayerGroup& middle = MAMiddleLayerGroup::Default ();
            middle.Register (this);
            LOG_WHEN (m_verbose, "Node " << GetNode ()->GetId () << " become " << m_middleHops << " hop middle layer at "
                << Simulator::Now ().GetMilliSeconds () << "ms");
        }
        
    }
}

void 
HumanApplication::CreateSendSocket ()
{
    NS_LOG_FUNCTION (this);
    if (m_socket) return;
    m_socket = Socket::CreateSocket (GetNode (), m_tid);
    m_socket->Bind ();
    m_socket->Connect (m_peer);
    m_socket->ShutdownRecv ();
    m_socket->SetAllowBroadcast (true);
}

void 
HumanApplication::CreateRecvSink ()
{
    NS_LOG_FUNCTION (this);
    if (m_sinkSocket) return;
    m_sinkSocket = Socket::CreateSocket (GetNode (), m_tid);
    // InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 9);
    m_sinkSocket->Bind (m_peer);
    m_sinkSocket->SetRecvCallback (MakeCallback (
        &HumanApplication::ReceivePacket, this));
}

void 
HumanApplication::ScheduleFirstTx ()
{
    NS_LOG_FUNCTION (this);
    m_scheduleEvent = Simulator::Schedule (MilliSeconds (m_startDelay),
        &HumanApplication::StartSending, this);
}

void
HumanApplication::StartSending ()
{
    NS_LOG_FUNCTION (this);
    TL_EVENT_LOG ("Node " << GetNode ()-> GetId () << " start sending");
    SendPacket ();
}

void 
HumanApplication::SendPacket ()
{
    NS_LOG_FUNCTION (this);
    NS_ASSERT (m_sendEvent.IsExpired ());
    
    if ((m_status == Status::Middle && m_tdmaEnabled && config.enableMiddleLayer) 
        || (m_status == Status::Outter && config.enableOuterLayer))
    {
        Ptr<Packet> packet = Create<Packet> (m_pktSize);
        PacketIdentityTag tag;
        tag.AssignUniqueId ();
        packet->AddByteTag (tag);

        Ptr<Ipv4> ipv4 = GetNode  ()->GetObject<Ipv4> ();
        m_txTrace (packet, ipv4->GetAddress (1, 0).GetLocal ());
        m_socket->Send (packet);
    }

    ScheduleNextTx ();
}

void 
HumanApplication::ScheduleNextTx ()
{
    NS_LOG_FUNCTION (this);
    Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
    var->SetStream (m_stream);
    double nextTime = m_beaconInterval * var ->GetValue (0.9, 1.1);
    m_sendEvent = Simulator::Schedule (MilliSeconds (nextTime),
        &HumanApplication::SendPacket, this);
}

void
HumanApplication::CancelAllEvents ()
{
    m_sendEvent.Cancel ();
    m_scheduleEvent.Cancel ();
    m_statusCheckEvent.Cancel ();
}

void 
HumanApplication::PeriodicCheckStatus ()
{
    if (m_status == Status::Middle 
        && (Simulator::Now () - m_lastTimeFromCore).GetMilliSeconds () > m_setOutInterval)
    {
        TL_EVENT_LOG ("Node " << GetNode ()-> GetId ()
            << " leave middle to outter layer"
            << ", last receive core packet at " 
            << m_lastTimeFromCore.GetSeconds ());
        LOG_WHEN (m_verbose, "\tNode " << GetNode ()-> GetId () << "leave middle at " << Simulator::Now ().GetMilliSeconds () << "ms");
        m_status = Status::Outter;
        // 从MiddleLayerGroup中去除
        MAMiddleLayerGroup& middle = MAMiddleLayerGroup::Default ();
        middle.Unregister (this);
    }
    m_statusCheckEvent = Simulator::Schedule (MilliSeconds (m_checkPeriod),
        &HumanApplication::PeriodicCheckStatus, this);
}

void
HumanApplication::SetTDMAEnable (bool val)
{
    m_tdmaEnabled = val;
}

}
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "core-layer.h"
#include "ns3/internet-module.h"
#include "ns3/wave-module.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/wifi-module.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("CoreLayerApplication");
NS_OBJECT_ENSURE_REGISTERED (CoreLayerApplication);

TypeId
CoreLayerApplication::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::CoreLayerApplication")
        .SetParent<Application> ()
        .AddConstructor<CoreLayerApplication> ()
        .AddAttribute ("DataRate", "Data rate",
            DataRateValue (DataRate ("500kb/s")),
            MakeDataRateAccessor (&CoreLayerApplication::m_dataRate),
            MakeDataRateChecker ())
        .AddAttribute ("Remote", "The Address of the destination",
            AddressValue (),
            MakeAddressAccessor (&CoreLayerApplication::m_peer),
            MakeAddressChecker ())
        .AddAttribute ("PacketSize", "Size of every packet",
            UintegerValue (512),
            MakeUintegerAccessor (&CoreLayerApplication::m_pktSize),
            MakeUintegerChecker<uint64_t> (0))
        .AddAttribute ("Protocol", "The Type of protocol to use, This should be "
            "a subclass of ns3::SocketFactory", 
            TypeIdValue (UdpSocketFactory::GetTypeId ()),
            MakeTypeIdAccessor (&CoreLayerApplication::m_tid),
            MakeTypeIdChecker ())
        .AddTraceSource ("Tx", "A new packet is created and is sent", 
            MakeTraceSourceAccessor (&CoreLayerApplication::m_txTrace),
            "ns3::Packet::AddressTraceCallback")
        .AddTraceSource ("Rx", "A new packet is received by sink",
            MakeTraceSourceAccessor (&CoreLayerApplication::m_rxTrace),
            "ns3::ThreeLayerHelper::TracedCallback")
    ;
    return tid;
}


CoreLayerApplication::CoreLayerApplication ()
  : m_slotId (0),
    m_socket (0),
    m_verbose (false),
    m_enabled (false),
    config (MyGlobalConfig::Default ()),
    m_slotHelper (NULL),
    m_appStarted (false)
{
    NS_LOG_FUNCTION (this);
}

CoreLayerApplication::~CoreLayerApplication ()
{
    NS_LOG_FUNCTION (this);
}

void
CoreLayerApplication::DoDispose ()
{
    NS_LOG_FUNCTION (this);
    m_socket = NULL;
    Application::DoDispose ();
}

void
CoreLayerApplication::StartSending ()
{
    NS_LOG_FUNCTION (this);
    ScheduleNextTx ();
    // Enable mac
    Ptr<WifiNetDevice> dev = dynamic_cast <WifiNetDevice*> (PeekPointer (GetNode ()->GetDevice (0))); //未看懂，要看下Wi-Fi模块了
    if (dev)
    {
        Ptr<OcbWifiMac> mac = dynamic_cast <OcbWifiMac*> (PeekPointer (dev->GetMac ()));
        if (mac) mac->Resume (); //继续，重新开始
    }
}

void
CoreLayerApplication::StopSending ()
{
    NS_LOG_FUNCTION (this);
    CancelEvents ();
    Ptr<WifiNetDevice> dev = dynamic_cast <WifiNetDevice*> (PeekPointer (GetNode ()->GetDevice (0)));
    if (dev)
    {
        Ptr<OcbWifiMac> mac = dynamic_cast <OcbWifiMac*> (PeekPointer (dev->GetMac ()));
        if (mac) mac->Suspend ();
    }
}

void 
CoreLayerApplication::CancelEvents ()
{
    Simulator::Cancel (m_sendEvent);
}

void 
CoreLayerApplication::ScheduleNextTx ()
{
    uint32_t bits = m_pktSize * 8;
    Time nextTime (Seconds (bits / static_cast<double>(m_dataRate.GetBitRate ()))); //长度除以速率
    if (m_enabled) {
        m_sendEvent = Simulator::Schedule (nextTime, &CoreLayerApplication::SendPacket, this);    
    } 
}

void
CoreLayerApplication::SendPacket ()
{
    NS_LOG_FUNCTION (this);
    NS_ASSERT (m_sendEvent.IsExpired ());
    if (config.enableCoreLayer && m_enabled) {
        Ptr<Packet> packet = Create<Packet> (m_pktSize);
        PacketIdentityTag tag;
        tag.AssignUniqueId ();
        packet->AddByteTag (tag);
        Ptr<Ipv4> ipv4 = GetNode ()->GetObject<Ipv4> ();
        m_txTrace (packet, ipv4->GetAddress (1, 0).GetLocal ());
        m_socket->Send (packet);
    }
    ScheduleNextTx ();
}

void
CoreLayerApplication::StartApplication ()
{
    NS_LOG_FUNCTION (this);
    m_appStarted = true;
    TryCreateSockets ();
}

void
CoreLayerApplication::TryCreateSockets ()
{
    if (!m_socket)
    {
        m_socket = Socket::CreateSocket (GetNode (), m_tid);
        if (Inet6SocketAddress::IsMatchingType (m_peer))
        {
            if (m_socket->Bind6 () == -1)
            {
                NS_FATAL_ERROR ("Fail to bind socket");
            }
        }
        else if (InetSocketAddress::IsMatchingType (m_peer) || 
                 PacketSocketAddress::IsMatchingType (m_peer))  
        {
            if (m_socket->Bind () == -1)
            {
                NS_FATAL_ERROR ("Fail to bind socket");
            }
        }

        m_socket->Connect (m_peer);
        m_socket->SetAllowBroadcast (true);
        m_socket->ShutdownRecv ();
        m_socket->SetDataSentCallback (MakeCallback (&CoreLayerApplication::PacketSentCallback, this)); //做啥子的？
    }

    if (!m_sinkSocket)
    {
        m_sinkSocket = Socket::CreateSocket (GetNode (), m_tid);
        m_sinkSocket->Bind (m_peer);
        m_sinkSocket->SetRecvCallback (MakeCallback (
            &CoreLayerApplication::ReceivePacket, this));
    }
}

void 
CoreLayerApplication::PacketSentCallback (Ptr<Socket> socket, uint32_t bytesSent)
{
    // NS_LOG_INFO (Simulator::Now ().GetSeconds () << " " 
    //     << GetNode ()->GetId () << " sent " << bytesSent << " bytes") ;
}

void
CoreLayerApplication::StopApplication ()
{
    NS_LOG_FUNCTION (this);
    m_appStarted = false;
    TL_EVENT_LOG ("Core Application of Node " << GetNode ()->GetId () << " stop");
    CancelEvents ();
    if (m_socket != 0)
    {
        m_socket->Close ();
    }
}

bool
CoreLayerApplication::IsCoreLayerApplicationInstalled (Ptr<Node> node)
{
    uint32_t nApps = node->GetNApplications ();
    for (uint32_t idx = 0; idx < nApps; idx ++)
    {
        Ptr<Application> app = node->GetApplication (idx);
        if (dynamic_cast <CoreLayerApplication*> (PeekPointer (app))) //某个模块里的用法？
        {
                return true;
        }
    }
    return false;
}

void 
CoreLayerApplication::ReceivePacket (Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address srcAddr;
    while ((packet = socket->RecvFrom (srcAddr)))
    {
        InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom (srcAddr);
        m_rxTrace (packet, this, inetAddr.GetIpv4 ());
    }
}

// Override 
void 
CoreLayerApplication::SlotEnabled (uint32_t slotSize, MADynamicSlotHelper& slotHelper) 
{
    NS_LOG_FUNCTION (this);
    if (!m_appStarted || m_enabled) 
        return;
    LOG_WHEN (m_verbose, "Core Layer Node start " << GetNode ()->GetId () 
        << " at slot " << slotHelper.GetSlotCount ());
    TryCreateSockets ();
    m_enabled = true;
    StartSending ();
}

void
CoreLayerApplication::SlotDisabled (uint32_t slotSize, MADynamicSlotHelper& slotHelper) 
{
    NS_LOG_FUNCTION (this);
    if (!m_appStarted || !m_enabled) 
        return;
    LOG_WHEN (m_verbose, "Core Layer Node stop " << GetNode ()->GetId ()
        << " at slot " << slotHelper.GetSlotCount ());
    m_enabled = false;
    StopSending ();
}

void 
CoreLayerApplication::RegisteredToSlotHelper (MADynamicSlotHelper& slotHelper) 
{
    NS_LOG_FUNCTION (this);
    m_slotHelper = &slotHelper;
}

void
CoreLayerApplication::SetSlotId (uint32_t slotId)
{
    m_slotId = slotId;
}

uint32_t
CoreLayerApplication::GetSlotId ()
{
    return m_slotId;
}

uint32_t 
CoreLayerApplication::MaxAllowedSlots ()
{
    return 1;
}

}

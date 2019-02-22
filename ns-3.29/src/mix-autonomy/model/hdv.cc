/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hdv.h"
#include "ap.h"

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
}

HumanApplication::~HumanApplication ()
{
    NS_LOG_FUNCTION (this);
}

void
HumanApplication::DoDispose ()
{
    NS_LOG_FUNCTION (this);
}

void 
HumanApplication::StartApplication ()
{
    NS_LOG_FUNCTION (this);
}


void 
HumanApplication::StopApplication ()
{
    NS_LOG_FUNCTION (this);
}

HumanApplication::Status
HumanApplication::GetStatus ()
{
    return m_status;
}

void
HumanApplication::SetTDMAEnable (bool val)
{
    m_tdmaEnabled = val;
}

void
HumanApplication::AddToMiddle ()
{
    m_status = Middle;
} 

void 
HumanApplication::QuitFromMiddle ()
{

}

void 
HumanApplication::ReceivePacket (Ptr<Packet> pkt, Address & srcAddr)
{
    InetSocketAddress addr = InetSocketAddress::ConvertFrom (srcAddr);
    Ipv4Address ipv4Addr = addr.GetIpv4 ();
    // 获取发送该数据包的节点
    Ptr<Node> node = GetNodeFromAddress (ipv4Addr);
    if (IsAPApplicationInstalled (node))
    {
        // 收到了来自内核层的数据包
        AddToMiddle ();
    }
    else
    {
        
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

}
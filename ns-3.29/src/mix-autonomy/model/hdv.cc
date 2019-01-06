/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hdv.h"

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

Status
HumanApplication::GetStatus ()
{
    return m_status;
}

void
HumanApplication::SetTDMAEnable (bool val)
{
    m_tdmaEnabled = val;
}

}
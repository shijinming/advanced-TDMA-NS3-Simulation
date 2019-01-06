/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ap.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("AutonomyApplication");
NS_OBJECT_ENSURE_REGISTERED (AutonomyApplication);

TypeId
AutonomyApplication::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::AutonomyApplication")
        .SetParent<Application> ()
        .AddConstructor<AutonomyApplication> ()
    ;
    return tid;
}

AutonomyApplication::AutonomyApplication ()
{
    NS_LOG_FUNCTION (this);
}

AutonomyApplication::~AutonomyApplication ()
{
    NS_LOG_FUNCTION (this);
}

void
AutonomyApplication::DoDispose ()
{
    NS_LOG_FUNCTION (this);
    Application::DoDispose ();
}

void
AutonomyApplication::StartApplication ()
{
    NS_LOG_FUNCTION (this);
    TryCreateSockets ();
}
 
void
AutonomyApplication::StopApplication ()
{
    NS_LOG_FUNCTION (this);
}

}

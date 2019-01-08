/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef AP_H
#define AP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3
{

class AutonomyApplication : public Application 
{
public:
    static TypeId GetTypeId (void);
    AutonomyApplication ();
    virtual ~AutonomyApplication (); //虚析构函数
    
protected:
    virtual void DoDispose (void);

private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);
};

}

#endif
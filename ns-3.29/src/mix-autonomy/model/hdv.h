/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef HDV_H
#define HDV_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3
{

class HumanApplication : public Application
{
public:
    static TypeId GetTypeId (void);
    HumanApplication ();
    virtual ~HumanApplication ();

    enum Status {Middle, Outter};
    //static int GetHumanStatus (Ptr<Node> node);
    Status GetStatus ();
    void SetTDMAEnable (bool val);

protected:
    virtual void DoDispose ();

private:
    virtual void StartApplication ();
    virtual void StopApplication ();

    bool                m_tdmaEnabled;  //只对中间层节点有效,true时意味着此节点可以在tdma时隙发送数据包
    Status              m_status;

};

}

#endif
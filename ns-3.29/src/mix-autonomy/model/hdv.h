/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef HDV_H
#define HDV_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "my-header.h"

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

    void ListenChannel(); //侦听信道
    void ParseHeader();  //解析内核层packet header
    void AddToMiddle(); //如果收到内核层包，更新状态信息
    void QuitFromMiddle(); //如果在一帧时间内的内核层时隙没有收到任何一个内核层的包，则离开中间层，更新状态信息

    void ReceivePacket (Ptr<Socket> socket);
    void ReceiveFromAP (Ptr<Packet> pkt, Ptr<Node> node);
    Ptr<Node> GetNodeFromAddress (Ipv4Address & address);
    bool IsAPApplicationInstalled (Ptr<Node> node);

protected:
    virtual void DoDispose ();

private:
    virtual void StartApplication ();
    virtual void StopApplication ();

    bool                m_tdmaEnabled;  //只对中间层节点有效,true时意味着此节点可以在tdma时隙发送数据包
    Status              m_status;

    /** 发送的trace */
    TracedCallback<Ptr<const Packet>, const Address & > m_txTrace;
    /** 接收的trace */
    TracedCallback<Ptr<const Packet>, Ptr<const Application>, const Address & > m_rxTrace;
};

}

#endif
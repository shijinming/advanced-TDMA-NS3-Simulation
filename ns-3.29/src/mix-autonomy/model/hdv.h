/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef HDV_H
#define HDV_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "my-header.h"
#include "tdma-app.h"
#include "ns3/sim-config.h"

namespace ns3
{

class HumanApplication : public TDMAApplication
{
public:
    static TypeId GetTypeId (void);
    HumanApplication ();
    virtual ~HumanApplication ();

    enum Status {Middle, Outter};
    //static int GetHumanStatus (Ptr<Node> node);
    int GetStatus ();
    void SetTDMAEnable (bool val);

    void AddToMiddle(); //如果收到内核层包，更新状态信息
    void QuitFromMiddle(); //如果在一帧时间内的内核层时隙没有收到任何一个内核层的包，则离开中间层，更新状态信息

    void ReceivePacket (Ptr<Packet> pkt, Address & srcAddr);
    void ReceiveFromAP (Ptr<Packet> pkt, Ptr<Node> node);
    Ptr<Node> GetNodeFromAddress (Ipv4Address & address);
    bool IsAPApplicationInstalled (Ptr<Node> node);
  
    virtual void SetupHeader(PacketHeader &hdr) {};
    void SlotAllocation () {};
    struct TDMASlot GetNextSlotInterval (void);

    void SendPacket (void);
    void SlotWillStart (void);

    void ChangeWindowSize (uint32_t cwMin, uint32_t cwMax, uint32_t channelNumber);

private:
    Status              m_status;
    uint64_t            receiveAPId;

    /** 发送的trace */
    TracedCallback<Ptr<const Packet>, const Address & > m_txTrace;
    /** 接收的trace */
    TracedCallback<Ptr<const Packet>, Ptr<const Application>, const Address & > m_rxTrace;
};

}

#endif
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef AP_APPLICATION_H
#define AP_APPLICATION_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include "ns3/sim-config.h"
#include "ns3/my-header.h"
#include "csma-app.h"

namespace ns3
{

class APFollower : public CSMAApplication
{
public:
  static TypeId GetTypeId(void);
  APFollower();
  virtual ~APFollower();
  void ReceivePacket(Ptr<Packet> pkt, Address &srcAddr); //判断接收到的包是否来自内核层
  bool ReceivePacketFromAP(Ptr<Packet> pkt);             //对接收到的leader的控制包进行处理解析
  void SetupHeader(PacketHeader &hdr);
  void SlotAllocation();
  void SendPacket(void);

private:
  uint64_t CCHSendSlot;
  std::vector<uint64_t> SCHSendSlot;
  uint32_t leaderPacketCnt;
};

} // namespace ns3

#endif
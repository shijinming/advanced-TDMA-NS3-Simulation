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

class APApplication : public CSMAApplication
{
public:
  static TypeId GetTypeId(void);
  APApplication();
  virtual ~APApplication();
  virtual void StartApplication(void);
  virtual void DoInitialize();
  virtual bool ReceivePacket(Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &srcAddr);
  virtual void ReceiveFromAP(Ptr<const Packet> pkt, uint16_t type);
  void SetupHeader(PacketHeader &hdr);
  void SlotAllocation();

private:
  uint64_t CCHSendSlot;
  std::vector<uint64_t> SCHSendSlot;
  uint32_t leaderPacketCnt;
  std::vector<uint16_t> m_CCHslotAllocation;
  std::vector<uint16_t> m_SCHslotAllocation;
  std::map<uint16_t, uint32_t> m_queueLen;
};

} // namespace ns3

#endif
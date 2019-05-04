/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef CSMA_APPLICATION_H
#define CSMA_APPLICATION_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wave-module.h"
#include "ns3/sim-config.h"
#include "my-header.h"

namespace ns3
{
class CSMAApplication : public Application
{
public:
  static TypeId GetTypeId(void);
  CSMAApplication();
  virtual ~CSMAApplication();

	virtual void StartApplication(void);
  virtual void StopApplication(void);
  virtual void DoDispose();
  virtual void DoInitialize();
  TracedCallback<Ptr<const Packet>, const Address &> txTrace;
  TracedCallback<Ptr<const Packet>, Ptr<const Application>, const Address &> rxTrace;
	void DoSendPacket(Ptr<Packet> pkt, uint32_t channel);
	void SendPacket(void);
  void WifiPhyTxBeginTrace(Ptr<const Packet> p);
  void GenerateTraffic(void);
  virtual bool ReceivePacket(Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &srcAddr);
  Ptr<Node> GetNodeFromAddress(const Address &address);
  virtual void ReceiveFromAP(Ptr<const Packet> pkt, uint16_t type);
  int GetVehicleType();
  virtual void StartCCH();
  void ChangeSCH();
  virtual void SetupHeader(PacketHeader &hdr) {}

	SimulationConfig &config;
  uint16_t m_type;
  std::queue<Ptr<Packet>> txq;
  bool m_isMiddle;
  Time lastTimeRecAP;
  Time startTxCCH;
  Time startTxSCH;
  Time m_gi;
  Time m_cchi;
  Time m_schi;
  Time m_synci;
  Time m_durationSCH;
  uint32_t m_SCH;
  Ptr<WaveNetDevice> m_device;
};

}

#endif
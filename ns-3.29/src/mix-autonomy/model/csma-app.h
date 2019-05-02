/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef CSMA_APP_H
#define CAMA_APP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"

#include "ns3/sim-config.h"

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
  void CreateSocket(void);
  void OnReceivePacket(Ptr<Socket> socket);
	void DoSendPacket(Ptr<Packet> pkt);
	void SendPacket(void);
  void WifiPhyTxBeginTrace(Ptr<const Packet> p);
  void generateTraffic(void);
  void ReceivePacket(Ptr<Packet> pkt, Address &srcAddr);
  Ptr<Node> GetNodeFromAddress(Ipv4Address &address);
  void ReceiveFromAP(Ptr<Packet> pkt, Ptr<Node> node);
  bool IsAPApplicationInstalled(Ptr<Node> node);
  void SwitchChannel();
  void SwitchToNextChannel(uint32_t curChannelNumber, uint32_t nextChannelNumber);

	SimulationConfig &config;
	Ptr<Socket> socket;
	Ptr<Socket> sink;
	TypeId socketTid;
	EventId sendEvent;
  std::queue<Ptr<Packet>> txqCCH;
  std::queue<Ptr<Packet>> txqSCH;
  bool m_isMiddle;
  Time lastTimeRecAP;
  Time startTxCCH;
  Time startTxSCH;
};

}

#endif
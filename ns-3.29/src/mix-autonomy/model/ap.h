/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef AP_H
#define AP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include "ns3/tdma-app.h"
#include "ns3/sim-config.h"
#include "ns3/my-header.h"

namespace ns3
{

class APFollower : public TDMAApplication
{
public:
  static TypeId GetTypeId (void);
  APFollower ();
  virtual ~APFollower ();
  void ReceivePacket (Ptr<Packet> pkt, Address & srcAddr); //对接收到的leader的控制包进行处理解析
  void ReceivePacketFromAP (Ptr<Packet> pkt);
  void SetupHeader(PacketHeader &hdr);
  bool IsAPApplicationInstalled (Ptr<Node> node);
  Ptr<Node> GetNodeFromAddress (Ipv4Address & address);
  void SlotAllocation () {};

private:
  uint64_t CCHSendSlot;
  std::vector <uint64_t> SCHSendSlot;
};

class APLeader : public APFollower
{
public:
  static TypeId GetTypeId (void);
  APLeader ();
  virtual ~APLeader ();
  void ReceivePacketFromAP (Ptr<Packet> pkt); //对接收到的follower的控制包进行处理解析
  /**
   * @brief 根据当前状态初始化发送包的帧头
   * 
   * @param hdr 
   */
  void SetupHeader(PacketHeader &hdr); //将时隙分配信息写入报头
  void SlotAllocation ();

private:
  //std::vector <struct PacketHeader::FrameHeader> followerList; //收到的follower控制包报头
  std::vector <uint16_t> m_CCHslotAllocation; //时隙分配数组，每个元素对应一个控制帧时隙，值为车辆id
  std::vector <uint16_t> m_SCHslotAllocation; //时隙分配数组，每个元素对应一个数据帧时隙，值为车辆id
  std::map <uint16_t, uint32_t> m_queueLen;
};

}

#endif
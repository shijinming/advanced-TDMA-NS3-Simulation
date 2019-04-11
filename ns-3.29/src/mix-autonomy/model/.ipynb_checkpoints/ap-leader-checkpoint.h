/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef AP_LEADER_H
#define AP_LEADER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include "ap-follower.h"
#include "ns3/sim-config.h"
#include "ns3/my-header.h"

namespace ns3
{

class APLeader : public APFollower
{
public:
  static TypeId GetTypeId(void);
  APLeader();
  virtual ~APLeader();
  void ReceivePacket(Ptr<Packet> pkt, Address &srcAddr);
  void ReceivePacketFromAP(Ptr<Packet> pkt); //对接收到的follower的控制包进行处理解析，写入m_queueLen中
  /**
   * @brief 根据当前状态初始化发送包的帧头
   * 
   * @param hdr 
   */
  void SetupHeader(PacketHeader &hdr); //将时隙分配信息写入报头
  void SlotAllocation();
  void SlotDidEnd(void);

private:
  //std::vector <struct PacketHeader::FrameHeader> followerList; //收到的follower控制包报头
  std::vector<uint16_t> m_CCHslotAllocation; //时隙分配数组，每个元素对应一个控制帧时隙，值为车辆id
  std::vector<uint16_t> m_SCHslotAllocation; //时隙分配数组，每个元素对应一个数据帧时隙，值为车辆id
  std::map<uint16_t, uint32_t> m_queueLen;
  std::vector<uint64_t> mySendSlot;
};

} // namespace ns3

#endif
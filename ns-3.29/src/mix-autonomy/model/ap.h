/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef AP_H
#define AP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include <vector>

#include "ns3/tdma-app.h"
#include "ns3/sim-config.h"
#include "ns3/my-header.h"

namespace ns3
{

class APFollower : public TDMAApplication
{
public:
void ReceivePacket (Ptr<const Packet> pkt, const Address & srcAddr); //对接收到的leader的控制包进行处理解析
private:

};

class APLeader : public TDMAApplication
{
public:
  void ReceivePacket (Ptr<const Packet> pkt, const Address & srcAddr); //对接收到的follower的控制包进行处理解析
  void SSHSlotAllocation(); //为AP和中间层分配数据帧时隙
  void CCHSlotAllocation(); //AP内控制帧时隙管理
  //void WriteAllocationToPacket(uint32_t* slotAllocation); //将时隙分配信息写入payload并加入发送队列
  
  /**
   * @brief 根据当前状态初始化发送包的帧头
   * 
   * @param hdr 
   */
  void SetupHeader(AllocationHeader &hdr); //将时隙分配信息写入报头

private:
  std::vector <struct FrameHeader> followerList; //收到的follower控制包报头
  std::vector <uint32_t> m_CCHslotAllocation; //时隙分配数组，每个元素对应一个控制帧时隙，值为车辆id
  std::vector <uint32_t> m_SCHslotAllocation; //时隙分配数组，每个元素对应一个数据帧时隙，值为车辆id
};

}

#endif
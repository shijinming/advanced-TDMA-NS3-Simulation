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
  void AddToAP();  //新的自动驾驶汽车加入AP,发送信息告诉leader
  void QuitFromAP(); //自动驾驶汽车离开AP

private:

};

class APLeader : public TDMAApplication
{
public:
  void SlotAllocation(); //为AP和中间层分配时隙
  void WriteAllocationToPacket(uint32_t* slotAllocation); //将时隙分配信息写入payload并加入发送队列

private:
  std::vector <struct FrameHeader> followerList;
  uint32_t* m_slotAllocation; //时隙分配数组，每个元素对应一个时隙，值为车辆id
};

}

#endif
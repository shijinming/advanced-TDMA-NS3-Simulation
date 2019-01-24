/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef AP_H
#define AP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include <vector>

namespace ns3
{

class AutonomyApplication : public Application 
{
public:
  static TypeId GetTypeId (void);
  AutonomyApplication ();
  virtual ~AutonomyApplication (); //虚析构函数

  void AddToAP();  //新的自动驾驶汽车加入AP,发送信息告诉leader
  void QuitFromAP(); //自动驾驶汽车离开AP

  uint32_t GetLocationLon();
    
protected:
  virtual void DoDispose (void);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void CreateSocket (void);

  /**
   * 在接收到数据包之后的回调函数
   */ 
  void OnReceivePacket ();
};

class APLeader : public AutonomyApplication
{
public:
  struct Follower
  {
    uint32_t id;
    uint32_t locLon;
    uint32_t locLat;
    uint32_t timestamp;
    uint32_t queueLen;
  };
  void SlotAllocation(); //为AP和中间层分配时隙
  void WriteAllocationToPacket(uint32_t* slotAllocation); //将时隙分配信息写入payload并加入发送队列
private:
  std::vector <struct Follower> followerList;
  uint32_t m_follwerNum;
  uint32_t m_slotNum; //每一帧内时隙数
  uint32_t* m_slotAllocation; //时隙分配数组，每个元素对应一个时隙，值为车辆id
};

}

#endif
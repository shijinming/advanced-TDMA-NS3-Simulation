/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef MY_HEADER_H
#define MY_HEADER_H

#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"

#include <iostream>

namespace ns3
{
/**
* @brief PacketHeader用于除leader以外车辆的报头
*
*/
class PacketHeader : public Header 
{
public:

  PacketHeader ();
  ~PacketHeader ();

  struct FollowerHeader {
    uint8_t type;     //车子类型，无人驾驶车or有人驾驶车
    uint16_t id;      //车辆编号
    uint16_t queueLen; //预计发送时长，决定数据帧中所分配时隙的个数if queueLen > 0, request for slot automatically
    uint32_t timestamp; //当前仿真时间
    uint32_t locLon;   //车辆位置经度
    uint32_t locLat;   //车辆位置纬度
  };
  struct LeaderHeader {
    uint8_t type;     //车子类型，无人驾驶车or有人驾驶车
    uint16_t id;      //车辆编号
    uint16_t queueLen; //预计发送时长，决定数据帧中所分配时隙的个数if queueLen > 0, request for slot automatically
    uint32_t timestamp; //当前仿真时间
    uint32_t locLon;   //车辆位置经度
    uint32_t locLat;   //车辆位置纬度
    uint16_t CCHSlotNum;
    uint16_t SCHSlotNum;
  };

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  void Print (std::ostream &os) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  uint32_t GetSerializedSize (void) const;
  uint16_t GetHeaderSize(void) 
  {
    uint16_t headerSize;
    if(m_isLeader)
    {
      headerSize = sizeof(LeaderHeader) + (m_data.CCHSlotNum + m_data.SCHSlotNum) * sizeof(uint16_t);
    }
    else
    {
      headerSize = sizeof(FollowerHeader);
    }
    return headerSize;
  };

  void SetType(uint8_t t) {m_data.type = t;}
  uint8_t GetType() {return m_data.type;}
  void SetId(uint16_t id) {m_data.id = id;}
  uint16_t GetId() {return m_data.id;}
  void SetQueueLen(uint16_t queueLen) {m_data.queueLen = queueLen;}
  uint16_t GetQueueLen() {return m_data.queueLen;}
  void SetTimestamp(uint32_t timestamp) {m_data.timestamp = timestamp;}
  uint32_t GetTimestamp() {return m_data.timestamp;}
  void SetLocLon(uint32_t locLon) {m_data.locLon = locLon;}
  uint32_t GetLocLon() {return m_data.locLon;}
  void SetLocLat(uint32_t locLat) {m_data.locLat = locLat;}
  uint32_t GetLocLat() {return m_data.locLat;}
  void SetCCHslotAllocation(std::vector <uint16_t> &CCHslotAllocation) 
  {
    m_data.CCHSlotNum = CCHslotAllocation.size();
    m_CCHslotAllocation = new uint16_t [m_data.CCHSlotNum];
    for(int i = 0;i < m_data.CCHSlotNum; i++)
      m_CCHslotAllocation[i] = CCHslotAllocation[i];
  }
  uint16_t* GetCCHslotAllocation() {return m_CCHslotAllocation;}
  void SetSCHslotAllocation(std::vector <uint16_t> &SCHslotAllocation)
  {
    m_data.SCHSlotNum = SCHslotAllocation.size();
    m_SCHslotAllocation = new uint16_t [m_data.SCHSlotNum];
    for(int i = 0;i < m_data.SCHSlotNum; i++)
      m_SCHslotAllocation[i] = SCHslotAllocation[i];
  }
  uint16_t* GetSCHslotAllocation() {return m_SCHslotAllocation;}
  void SetIsLeader(bool isLeader) 
  {
    m_isLeader = isLeader;
  }
  bool GetIsLeader() {return m_isLeader;}

private:
  LeaderHeader m_data;  //!< Header data
  uint16_t m_headerSize;
  uint16_t *m_CCHslotAllocation; //分配控制帧时隙，值为车辆id
  uint16_t *m_SCHslotAllocation; //分配数据帧时隙，值为车辆id
  bool m_isLeader;
};

}

#endif
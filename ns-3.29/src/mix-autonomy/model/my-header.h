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
    uint16_t slotId;   //当前为仿真第几个时隙
    uint32_t slotSize; //仿真时隙的长度，tdma中为恒定值
  };

  struct LeaderHeader {
    uint8_t type;     //车子类型，无人驾驶车or有人驾驶车
    uint16_t id;      //车辆编号
    uint16_t queueLen; //预计发送时长，决定数据帧中所分配时隙的个数 if queueLen > 0, request for slot automatically
    uint32_t timestamp; //当前仿真时间
    uint32_t locLon;   //车辆位置经度
    uint32_t locLat;   //车辆位置纬度
    uint16_t slotId;   //当前为仿真第几个时隙
    uint32_t slotSize; //仿真时隙的长度，tdma中为恒定值
    std::vector <uint32_t> CCHslotAllocation; //分配控制帧时隙，值为车辆id
    std::vector <uint32_t> SCHslotAllocation; //分配数据帧时隙，值为车辆id
  };

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  void Print (std::ostream &os) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  uint32_t GetSerializedSize (void) const;
  uint16_t GetHeaderSize(void) {return m_headerSize;};

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
  void SetSlotId(uint16_t slotId) {m_data.slotId = slotId;}
  uint16_t GetSlotId() {return m_data.slotId;}
  void SetSlotSize(uint32_t slotSize) {m_data.slotSize = slotSize;}
  uint32_t GetSlotSize() {return m_data.slotSize;}
  void SetCCHslotAllocation(std::vector <uint32_t> &CCHslotAllocation) {m_data.CCHslotAllocation = CCHslotAllocation;}
  std::vector <uint32_t> GetCCHslotAllocation() {return m_data.CCHslotAllocation;}
  void SetSCHslotAllocation(std::vector <uint32_t> &SCHslotAllocation) {m_data.SCHslotAllocation = SCHslotAllocation;}
  std::vector <uint32_t> GetSCHslotAllocation() {return m_data.SCHslotAllocation;}
  void SetIsLeader(bool isLeader) 
  {
    m_isLeader = isLeader;
    if(isLeader)
      m_headerSize = sizeof(LeaderHeader);
  }
  bool GetIsLeader() {return m_isLeader;}

private:
  LeaderHeader m_data;  //!< Header data
  uint16_t m_headerSize;
  bool m_isLeader;
};

}

#endif
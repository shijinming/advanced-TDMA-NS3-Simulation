/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef MY_HEADER_H
#define MY_HEADER_H

#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"

#include <iostream>

namespace ns3
{

class PacketHeader : public Header 
{
public:

  PacketHeader ();
  ~PacketHeader ();

  typedef struct FrameHeader {
    uint8_t type;
    uint16_t id;
    uint16_t queueLen;   //length of send queue, if queueLen > 0, request for slot automatically
    uint32_t timestamp;
    uint32_t locLon;   //the longitude of vehicle
    uint32_t locLat;   //the latitude of vehicle
    uint16_t slotId;
    uint32_t slotSize;
    uint32_t priority; //发送类型优先级，决定所分配时隙的先后
    uint32_t sendDuration; //预计发送时长
  } FrameHeader;


  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  void Print (std::ostream &os) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  uint32_t GetSerializedSize (void) const;
  
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
  void SetPriority(uint32_t priority) {m_data.priority = priority;}
  uint32_t GetPriority() {return m_data.priority;}
  void SetSendDuration(uint32_t sendDuration) {m_data.sendDuration = sendDuration;}
  uint32_t GetSendDuration() {return m_data.sendDuration;}

private:
  FrameHeader m_data;  //!< Header data
  uint16_t m_headerSize;
};

}

#endif
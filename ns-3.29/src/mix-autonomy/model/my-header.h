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

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  void Print (std::ostream &os) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  uint32_t GetSerializedSize (void) const;
  
  void setIsAP(uint8_t isAP) {m_isAP = isAP;}
  uint8_t getIsAP() {return m_isAP;}
  void setIsMiddle(uint8_t isMiddle) {m_isMiddle = isMiddle;}
  uint8_t getIsMiddle() {return m_isMiddle;}
  void setId(uint16_t id) {m_id = id;}
  uint16_t getId() {return m_id;}
  void setQueueLen(uint32_t queueLen) {m_queueLen = queueLen;}
  uint32_t getQueueLen() {return m_queueLen;}
  void setTimestamp(uint32_t timestamp) {m_timestamp = timestamp;}
  uint32_t getTimestamp() {return m_timestamp;}
  void setLocLon(uint32_t locLon) {m_locLon = locLon;}
  uint32_t getLocLon() {return m_locLon;}
  void setLocLat(uint32_t locLat) {m_locLat = locLat;}
  uint32_t getLocLat() {return m_locLat;}
  void setSlotId(uint16_t slotId) {m_slotId = slotId;}
  uint16_t getSlotId() {return m_slotId;}
  void setSlotSize(uint16_t slotSize) {m_slotSize = slotSize;}
  uint16_t getSlotSize() {return m_slotSize;}

private:
  uint16_t m_headerSize;
  uint8_t m_isAP;
  uint8_t m_isMiddle;
  uint16_t m_id;
  uint16_t m_queueLen;   //length of send queue, if queueLen > 0, request for slot automatically
  uint32_t m_timestamp;
  uint32_t m_locLon;   //the longitude of vehicle
  uint32_t m_locLat;   //the latitude of vehicle
  uint16_t m_slotId;
  uint32_t m_slotSize;
};

}

#endif
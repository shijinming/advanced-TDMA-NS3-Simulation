/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "my-header.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

namespace ns3
{
PacketHeader::PacketHeader ()
{
  m_isLeader = false;
}

PacketHeader::~PacketHeader ()
{
  if (m_isLeader)
  {
    delete [] m_CCHslotAllocation;
    delete [] m_SCHslotAllocation;
  }
}

TypeId
PacketHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PacketHeader")
    .SetParent<Header> ()
    .SetGroupName ("mix-autonomy")
    .AddConstructor<PacketHeader> ()
  ;
  return tid;
}
TypeId
PacketHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
PacketHeader::Print (std::ostream &os) const
{
  // This method is invoked by the packet printing
  // routines to print the content of my header.

  std::ostringstream oss1;
  std::ostringstream oss2;

#define QUICK_PRINT(x) oss1 << #x << "\t"; oss2 << m_data.x << "\t"
  
  QUICK_PRINT (type);
  QUICK_PRINT (id);
  QUICK_PRINT (queueLen);
  QUICK_PRINT (locLon);
  QUICK_PRINT (locLat);
  QUICK_PRINT (timestamp);  // 时间戳长度最长，放在后面
  QUICK_PRINT (CCHSlotNum);
  QUICK_PRINT (SCHSlotNum);

#undef QUICK_PRINT
  os << oss1.str () << std::endl << oss2.str () << std::endl;
}

uint32_t
PacketHeader::GetSerializedSize (void) const
{
  // we reserve 2 bytes for our header.
  if(m_isLeader)
  {
    return sizeof(LeaderHeader) + (m_data.CCHSlotNum + m_data.SCHSlotNum) * sizeof(uint16_t) + sizeof(uint16_t);
  }
  else
  {
    return sizeof(FollowerHeader) + sizeof(uint16_t);
  }
}
void
PacketHeader::Serialize (Buffer::Iterator start) const
{
  // we can serialize two bytes at the start of the buffer.
  // we write them in network byte order.
  if(m_isLeader)
  {
    start.WriteU16(sizeof(LeaderHeader) + (m_data.CCHSlotNum + m_data.SCHSlotNum) * sizeof(uint16_t));
    start.Write ((uint8_t *) &m_data, sizeof(LeaderHeader));
    start.Write ((uint8_t *) m_CCHslotAllocation, m_data.CCHSlotNum * sizeof(uint16_t));
    start.Write ((uint8_t *) m_SCHslotAllocation, m_data.SCHSlotNum * sizeof(uint16_t));
  }
  else
  {
    start.WriteU16 (sizeof(FollowerHeader));
    start.Write ((uint8_t *) &m_data, sizeof(FollowerHeader));
  }
  

}
uint32_t
PacketHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  uint16_t headerSize =  start.ReadU16();
  if(headerSize == sizeof(FollowerHeader))
  {
    m_isLeader = false;
    start.Read ((uint8_t *) &m_data, sizeof(FollowerHeader));
  }
  else
  {
    m_isLeader = true;
    start.Read ((uint8_t *) &m_data, sizeof(LeaderHeader));
    m_CCHslotAllocation = new uint16_t [m_data.CCHSlotNum];
    m_SCHslotAllocation = new uint16_t [m_data.SCHSlotNum];
    start.Read ((uint8_t *) m_CCHslotAllocation, m_data.CCHSlotNum * sizeof(uint16_t));
    start.Read ((uint8_t *) m_SCHslotAllocation, m_data.SCHSlotNum * sizeof(uint16_t));
  }

  // we return the number of bytes effectively read.
  return GetSerializedSize ();
}

}

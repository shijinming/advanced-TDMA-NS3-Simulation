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
  m_headerSize = sizeof(FrameHeader);
}

PacketHeader::~PacketHeader ()
{
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
  QUICK_PRINT (slotId);
  QUICK_PRINT (slotSize);
  QUICK_PRINT (timestamp);  // 时间戳长度最长，放在后面

#undef QUICK_PRINT

  os << oss1.str () << std::endl << oss2.str () << std::endl;
}

uint32_t
PacketHeader::GetSerializedSize (void) const
{
  // we reserve 2 bytes for our header.
  return m_headerSize + sizeof(m_headerSize);
}
void
PacketHeader::Serialize (Buffer::Iterator start) const
{
  // we can serialize two bytes at the start of the buffer.
  // we write them in network byte order.
  start.WriteU16(m_headerSize);
  start.Write ((uint8_t *) &m_data, sizeof (FrameHeader));

}
uint32_t
PacketHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  m_headerSize =  start.ReadU16();
  start.Read ((uint8_t *) &m_data, sizeof (FrameHeader));

  // we return the number of bytes effectively read.
  return GetSerializedSize ();
}


AllocationHeader::AllocationHeader ()
{
  m_headerSize = sizeof(LeaderHeader);
}

AllocationHeader::~AllocationHeader ()
{
}

TypeId
AllocationHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AllocationHeader")
    .SetParent<Header> ()
    .SetGroupName ("mix-autonomy")
    .AddConstructor<AllocationHeader> ()
  ;
  return tid;
}
TypeId
AllocationHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
AllocationHeader::Print (std::ostream &os) const
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
  QUICK_PRINT (slotId);
  QUICK_PRINT (slotSize);
  //QUICK_PRINT (CCHslotAllocation);
  //QUICK_PRINT (SCHslotAllocation);
  QUICK_PRINT (timestamp);  // 时间戳长度最长，放在后面

#undef QUICK_PRINT

  os << oss1.str () << std::endl << oss2.str () << std::endl;
}

uint32_t
AllocationHeader::GetSerializedSize (void) const
{
  // we reserve 2 bytes for our header.
  return m_headerSize + sizeof (uint16_t);
}
void
AllocationHeader::Serialize (Buffer::Iterator start) const
{
  // we can serialize two bytes at the start of the buffer.
  // we write them in network byte order.
  start.WriteU16(m_headerSize);
  start.Write ((uint8_t *) &m_data, m_headerSize);
}
uint32_t
AllocationHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  m_headerSize = start.ReadU16();
  start.Read ((uint8_t *) &m_data, m_headerSize);

  // we return the number of bytes effectively read.
  return GetSerializedSize();
}
}

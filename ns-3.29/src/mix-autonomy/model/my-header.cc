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

#define QUICK_PRINT(x) os << #x << ": " << m_data.x << std::endl

  os << "type" << ": " << int(m_data.type) << std::endl;
  QUICK_PRINT (id);
  QUICK_PRINT (queueLen);
  QUICK_PRINT (timestamp);
  QUICK_PRINT (locLon);
  QUICK_PRINT (locLat);
  QUICK_PRINT (slotId);
  QUICK_PRINT (slotSize);

#undef QUICK_PRINT
}

uint32_t
PacketHeader::GetSerializedSize (void) const
{
  // we reserve 2 bytes for our header.
  return m_headerSize;
}
void
PacketHeader::Serialize (Buffer::Iterator start) const
{
  // we can serialize two bytes at the start of the buffer.
  // we write them in network byte order.

  start.Write ((uint8_t *) &m_data, sizeof (FrameHeader));

}
uint32_t
PacketHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  start.Read ((uint8_t *) &m_data, sizeof (FrameHeader));

  // we return the number of bytes effectively read.
  return m_headerSize;
}

}

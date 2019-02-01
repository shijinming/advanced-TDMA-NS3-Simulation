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
  m_headerSize = 26;
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

  os<<m_isAP<<"\t"<<m_isMiddle<<"\t"<<m_id<<"\t"<<m_queueLen<<"\t";
  os<<m_timestamp<<"\t"<<m_locLon<<"\t"<<m_locLon<<"\t"<<m_slotId<<"\t";
  os<<m_slotSize<<std::endl;
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
  Buffer::Iterator i = start;
  i.WriteHtonU16(m_headerSize);
  i.WriteU8(m_isAP);
  i.WriteU8(m_isMiddle);
  i.WriteHtonU16(m_id);
  i.WriteHtonU16(m_queueLen);
  i.WriteHtonU32(m_timestamp);
  i.WriteHtonU32(m_locLon);
  i.WriteHtonU32(m_locLat);
  i.WriteHtonU16(m_slotId);
  i.WriteHtonU32(m_slotSize);

}
uint32_t
PacketHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  Buffer::Iterator i = start;
  m_headerSize = i.ReadNtohU16();
  m_isAP = i.ReadU8();
  m_isMiddle = i.ReadU8();
  m_id = i.ReadNtohU16();
  m_queueLen = i.ReadNtohU16();
  m_timestamp = i.ReadNtohU32();
  m_locLon = i.ReadNtohU32();
  m_locLat = i.ReadNtohU32();
  m_slotId = i.ReadNtohU16();
  m_slotSize = i.ReadNtohU16();

  // we return the number of bytes effectively read.
  return m_headerSize;
}

}

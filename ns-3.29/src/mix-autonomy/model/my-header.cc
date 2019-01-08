/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "my-header.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

namespace ns3
{
MyHeader::MyHeader ()
{
  // we must provide a public default constructor, 
  // implicit or explicit, but never private.
}
MyHeader::~MyHeader ()
{
}

void 
MyHeader::SetData (FrameHeader data)
{
  m_data = data;
}

MyHeader::FrameHeader
MyHeader::GetData (void) const
{
  return m_data;
}

TypeId
MyHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyHeader")
    .SetParent<Header> ()
    .AddConstructor<MyHeader> ()
  ;
  return tid;
}
TypeId
MyHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
MyHeader::Print (std::ostream &os) const
{
  // This method is invoked by the packet printing
  // routines to print the content of my header.
  //os << "data=" << m_data << std::endl;
  //os << "data=" << m_data;
}
uint32_t
MyHeader::GetSerializedSize (void) const
{
  // we reserve 2 bytes for our header.
  return 2;
}
void
MyHeader::Serialize (Buffer::Iterator start) const
{
  // we can serialize two bytes at the start of the buffer.
  // we write them in network byte order.
  //start.WriteHtonU16 (m_data);
}
uint32_t
MyHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  // m_data = start.ReadNtohU16 ();

  // we return the number of bytes effectively read.
  return 2;
}

}

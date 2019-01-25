/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef MY_HEADER_H
#define MY_HEADER_H

#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

namespace ns3
{

class MyHeader : public Header 
{
public:

  MyHeader ();
  virtual ~MyHeader ();

  typedef struct FrameHeader {
    bool isAP;
    bool isMiddle;
    uint16_t id;
    uint32_t queueLen;   //length of send queue, if queueLen > 0, request for slot automatically
    uint32_t timestamp;
    uint32_t locLon;   //the longitude of vehicle
    uint32_t locLat;   //the latitude of vehicle
    uint16_t slotId;
    uint16_t slotSize;
  } FrameHeader;

  /**
   * Set the header data.
   * \param data The data.
   */
  void SetData (FrameHeader data);
  /**
   * Get the header data.
   * \return The data.
   */
  FrameHeader GetData (void) const;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;
private:
  FrameHeader m_data;  //!< Header data
};

}

#endif
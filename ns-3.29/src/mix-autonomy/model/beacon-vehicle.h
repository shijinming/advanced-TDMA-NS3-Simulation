/**
 * @file beacon-vehicle.h
 * @author Woody Huang (woodyhuang1@gmail.com)
 * @brief 测试TDMAApplication的简单Beacon类
 * @version 0.1
 * @date 2019-01-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef BEACON_VEHICLE_H
#define BEACON_VEHICLE_H

#include "ns3/tdma_app.h"

namespace ns3
{

class BeaconVehicleApplication : public TDMAApplication {
public:
  static TypeId GetTypeId (void);
private:
  Time slotSize;

  virtual void ReceivePacket (Ptr<const Packet> pkt, const Address & srcAddr) {};
  virtual struct TDMASlot GetNextSlotInterval (void);
  virtual struct TDMASlot GetInitalSlot (void);
};

TypeId
BeaconVehicleApplication::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::BeaconVehicleApplication")
    .SetParent <TDMAApplication> ()
    .AddAttribute ("SlotSize", "Size of time slot",
      TimeValue (MilliSeconds (20)),
      MakeTimeAccessor (&BeaconVehicleApplication::slotSize),
      MakeTimeChecker ());
  return tid;
}

struct TDMASlot 
BeaconVehicleApplication::GetNextSlotInterval (void)
{
  struct TDMASlot slot;
  slot.start = slotSize * (config.nNodes - 1);
  slot.duration = slotSize;
  return slot;
}

struct
BeaconVehicleApplication::GetInitalSlot (void)
{
  struct TDMASlot slot;
  slot.start = slotSize * GetNode ()->GetId ();
  slot.duration =  slotSize;
  return slot;
}

}

#endif
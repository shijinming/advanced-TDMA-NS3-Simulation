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

#include "ns3/tdma-app.h"

namespace ns3
{

class BeaconVehicleApplication : public TDMAApplication {
public:
  static TypeId GetTypeId (void);
  BeaconVehicleApplication () {};
  virtual void ReceivePacket (Ptr<Packet> pkt, Address & srcAddr) {};
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
    .AddConstructor <BeaconVehicleApplication> ()
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
  LOG_UNCOND ("Get Next Slot " << GetNode ()->GetId ());
  slot.start = slotSize * (config.nNodes - 1) + minTxInterval;
  slot.duration = slotSize - minTxInterval;
  return slot;
}

struct TDMASlot
BeaconVehicleApplication::GetInitalSlot (void)
{
  struct TDMASlot slot;
  LOG_UNCOND ("Get Initial Slot " << GetNode ()->GetId ());
  slot.start = slotSize * GetNode ()->GetId () + minTxInterval;
  slot.duration =  slotSize - minTxInterval;
  return slot;
}

NS_OBJECT_ENSURE_REGISTERED (BeaconVehicleApplication);

}

#endif
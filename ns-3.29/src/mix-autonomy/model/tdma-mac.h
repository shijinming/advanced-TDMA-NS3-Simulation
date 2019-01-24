#ifndef TDMA_MAC_H
#define TDMA_MAC_H

#include "ns3/ocb-wifi-mac.h"

namespace ns3
{

class TdmaMac : public OcbWifiMac
{
public:
  void ListenChannel();
  void ParseHeader();

private:

};

}

#endif
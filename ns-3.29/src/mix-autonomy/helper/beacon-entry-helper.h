/**
 * @file beacon-entry-helper.h
 * @author Woody Huang (woodyhuang1@gmail.com)
 * @brief 这个类是用来测试tdma-app.h/c的工具类。
 * @version 0.1
 * @date 2019-01-11
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "ns3/network-module.h"
#include "ap-helper.h"
#include "hdv-helper.h"
#include "entry-helper.h"
#include "ns3/internet-module.h"

namespace ns3
{

class BeaconSimulationEntry: public SimulationEntry
{
public:
  void Simulate (int argc, char **argv) override;
protected:
  //void OverrideDefaultConfig () override;
  // void LoadMobilityData () override;
  void ConfigureApplication () override;

  void PrintSendPacket(Ptr<const Packet> packet, const Address &address);
  void PrintReceivePacket(Ptr<const Packet> packet, ns3::Ptr<ns3::Application const> app, const Address &address);
};

void
BeaconSimulationEntry::Simulate (int argc, char **argv)
{
  doValidate = false;
  SimulationEntry::Simulate (argc, argv);
}

/*
void
BeaconSimulationEntry::OverrideDefaultConfig ()
{
  config.nNodes = 3;
}
*/

/**
 * @brief 创建简单的拓扑
*/ 
/*
void
BeaconSimulationEntry::LoadMobilityData ()
{
  MobilityHelper m;
  LOG_UNCOND ("Set Constant Mobility Model");
  m.SetPositionAllocator ("ns3::GridPositionAllocator",
                          "MinX", DoubleValue (0.0),
                          "MinY", DoubleValue (0.0),
                          "DeltaX", DoubleValue (10),
                          "DeltaY", DoubleValue (0),
                          "GridWidth", UintegerValue (config.nNodes),
                          "LayoutType", StringValue ("RowFirst"));
  m.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // Install to all nodes
  for (auto i = NodeList::Begin (); i != NodeList::End (); i ++)
    {
      m.Install (*i);
    }
  LOG_UNCOND ("Done set mobility");
}
*/
void
BeaconSimulationEntry::ConfigureApplication ()
{
  APLeaderHelper apleaderHelper;
  APFollowerHelper apfollowerHelper;
  HumanApplicationHelper hdvHelper;
  //8辆自动驾驶车辆，其中一个为leader
  for (auto node = NodeList::Begin (); node != NodeList::End (); node ++) 
    { Ptr<Application> app;
      if ((*node)->GetId() == 0) 
        app = apleaderHelper.Install(*node);
      else if ((*node)->GetId() <= 7) 
        app = apfollowerHelper.Install(*node);
      else
        app = hdvHelper.Install(*node);
        app->TraceConnectWithoutContext ("Tx", MakeCallback (&BeaconSimulationEntry::PrintSendPacket, this));
        app->TraceConnectWithoutContext ("Rx", MakeCallback (&BeaconSimulationEntry::PrintReceivePacket, this));
      //(*node)->AddApplication (app);
    }
  LOG_UNCOND ("Done create application");
}

void 
BeaconSimulationEntry::PrintSendPacket(Ptr<const Packet> packet, const Address &address)
{
  // std::cout<<"Send a packet "<<packet<<" from "<<address<<std::endl;
  // std::cout<<address<<','<<packet->GetUid ()<<','<<address.GetNode()->GetObject<Mobility>()->GetPosition()<<std::endl;
}

void 
BeaconSimulationEntry::PrintReceivePacket(Ptr<const Packet> packet, ns3::Ptr<ns3::Application const> app, const Address &address)
{
  Ptr<Ipv4> ipv4 = app->GetNode ()->GetObject<Ipv4> ();
  // std::cout<<ipv4->GetAddress (1, 0).GetLocal ()<<" Received a packet "<<packet<<" from "<<address<<std::endl;
  // std::cout<<ipv4->GetAddress (1, 0).GetLocal ()<<','<<packet->GetUid ()<<','<<address<<','<<app->GetNode ()->GetObject<ConstantPositionMobilityModel>()->GetPosition()<<std::endl;
}

}
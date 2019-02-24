#include "ap-helper.h"

namespace ns3
{

APFollowerHelper::APFollowerHelper ()
{
  m_factory.SetTypeId("ns3::APFollower");
}

void
APFollowerHelper::SetAttribute (std::string name, const AttributeValue & value)
{
  m_factory.Set (name, value);
}

Ptr<Application>
APFollowerHelper::Install (Ptr<Node> node)  const
{
  return InstallPriv (node);
}

ApplicationContainer
APFollowerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i ++)
  {
    apps.Add (InstallPriv (*i));
  }
  return apps;
}

Ptr<Application>
APFollowerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);
  return app;
}

APLeaderHelper::APLeaderHelper ()
{
  m_factory.SetTypeId("ns3::APLeader");
}

void
APLeaderHelper::SetAttribute (std::string name, const AttributeValue & value)
{
  m_factory.Set (name, value);
}

Ptr<Application>
APLeaderHelper::Install (Ptr<Node> node)  const
{
  return InstallPriv (node);
}

ApplicationContainer
APLeaderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i ++)
  {
    apps.Add (InstallPriv (*i));
  }
  return apps;
}

Ptr<Application>
APLeaderHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);
  return app;
}

}
#include "hdv-helper.h"

namespace ns3 
{

HumanApplicationHelper::HumanApplicationHelper ()
{
    m_factory.SetTypeId ("ns3::HumanApplication");
}

void
HumanApplicationHelper::SetAttribute (std::string name, const AttributeValue & value)
{
    m_factory.Set (name, value);
}

Ptr<Application>
HumanApplicationHelper::Install (Ptr<Node> node) const
{
    return InstallPriv (node);
}

ApplicationContainer
HumanApplicationHelper::Install (NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i ++)
    {
        apps.Add (InstallPriv (*i));
    }
    return apps;
}

Ptr<Application>
HumanApplicationHelper::InstallPriv (Ptr<Node> node) const 
{
    Ptr<Application> app = m_factory.Create<Application> ();
    node->AddApplication (app);
    return app;
}

}
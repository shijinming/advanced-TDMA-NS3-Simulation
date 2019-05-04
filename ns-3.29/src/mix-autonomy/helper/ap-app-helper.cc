#include "ap-app-helper.h"

namespace ns3
{

APApplicationHelper::APApplicationHelper()
{
    m_factory.SetTypeId("ns3::APApplication");
}

void APApplicationHelper::SetAttribute(std::string name, const AttributeValue &value)
{
    m_factory.Set(name, value);
}

Ptr<Application>
APApplicationHelper::Install(Ptr<Node> node) const
{
    return InstallPriv(node);
}

ApplicationContainer
APApplicationHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); i++)
    {
        apps.Add(InstallPriv(*i));
    }
    return apps;
}

Ptr<Application>
APApplicationHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<Application>();
    node->AddApplication(app);
    return app;
}

} // namespace ns3
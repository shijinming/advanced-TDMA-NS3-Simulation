#include "csma-app-helper.h"

namespace ns3
{

CSMAApplicationHelper::CSMAApplicationHelper()
{
    m_factory.SetTypeId("ns3::CSMAApplication");
}

void CSMAApplicationHelper::SetAttribute(std::string name, const AttributeValue &value)
{
    m_factory.Set(name, value);
}

Ptr<Application>
CSMAApplicationHelper::Install(Ptr<Node> node) const
{
    return InstallPriv(node);
}

ApplicationContainer
CSMAApplicationHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); i++)
    {
        apps.Add(InstallPriv(*i));
    }
    return apps;
}

Ptr<Application>
CSMAApplicationHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<Application>();
    node->AddApplication(app);
    return app;
}

} // namespace ns3
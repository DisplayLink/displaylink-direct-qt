// Copyright (C) 2026 Synaptics Incorporated
#include <qpa/qplatformintegrationplugin.h>

#include "integration.hpp"

namespace dl::sdk::qpa::plugin
{

class IntegrationPlugin : public QPlatformIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid FILE "dlsdk.json")

public:
    QPlatformIntegration* create(const QString& system, const QStringList& params) override
    {
        if (!system.compare(QLatin1String("dlsdk"), Qt::CaseInsensitive))
        {
            return new Integration(params);
        }
        return nullptr;
    }
};

} // namespace dl::sdk::qpa::plugin

#include "main.moc"

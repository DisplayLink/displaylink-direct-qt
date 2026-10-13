// Copyright (C) 2026 Synaptics Incorporated
#pragma once
#include <QLoggingCategory>

namespace dl::sdk::qpa::plugin
{

Q_DECLARE_LOGGING_CATEGORY(g_lcPlugin);
Q_DECLARE_LOGGING_CATEGORY(g_lcDisplay);
Q_DECLARE_LOGGING_CATEGORY(g_lcInput);
Q_DECLARE_LOGGING_CATEGORY(g_lcDraw);

#define logDebug(category) qCDebug(category).noquote()
#define logInfo(category) qCInfo(category).noquote()
#define logWarning(category) qCWarning(category).noquote()
#define logCritical(category) qCCritical(category).noquote()

} // namespace dl::sdk::qpa::plugin

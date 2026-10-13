// Copyright (C) 2026 Synaptics Incorporated
#pragma once
#include <QRegion>

#include <vector>

#include <driver/types.hpp>

namespace dl::sdk::qpa::plugin
{

// Region must already be clipped to the screen, as the SDK takes unsigned coordinates
std::vector<core::DirtyRect> toDirtyRects(const QRegion& region, int maxDirtyRectCount);

} // namespace dl::sdk::qpa::plugin

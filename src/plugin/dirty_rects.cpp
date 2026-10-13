// Copyright (C) 2026 Synaptics Incorporated
#include "dirty_rects.hpp"

#include <QRect>

namespace dl::sdk::qpa::plugin
{

namespace
{

core::DirtyRect toDirtyRect(const QRect& rect)
{
    return core::DirtyRect{
        static_cast<unsigned int>(rect.x()),
        static_cast<unsigned int>(rect.y()),
        static_cast<unsigned int>(rect.width()),
        static_cast<unsigned int>(rect.height())
    };
}

} // namespace

std::vector<core::DirtyRect> toDirtyRects(const QRegion& region, int maxDirtyRectCount)
{
    if (region.rectCount() > maxDirtyRectCount)
    {
        return {toDirtyRect(region.boundingRect())};
    }

    std::vector<core::DirtyRect> rects;
    rects.reserve(static_cast<size_t>(region.rectCount()));
    for (const QRect& rect : region)
    {
        rects.push_back(toDirtyRect(rect));
    }
    return rects;
}

} // namespace dl::sdk::qpa::plugin

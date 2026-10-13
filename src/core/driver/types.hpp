// Copyright (C) 2026 Synaptics Incorporated
#pragma once
#include <span>
#include <vector>

#include <stdint.h>

namespace dl::sdk::qpa::core
{

// Mapped from dlsdk_status_t enum
enum class DriverStatus
{
    Success = 0,
    NotEnoughSpace,
    Unsuccessful,
    UnsuccessfulNoDevice,
    UnsuccessfulNoMonitor,
    UnsuccessfulMonitorOff,
    InvalidHandle,
    InvalidArgs,
    Timeout,
    NotImplemented,
    Busy,
};

struct DisplaySize
{
    unsigned int width = 0;
    unsigned int height = 0;
};

struct DirtyRect
{
    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int width = 0;
    unsigned int height = 0;
};

struct DisplayMode
{
    DisplaySize resolution;
    unsigned int refreshRateHz = 0;
};

struct Frame
{
    std::span<const uint8_t> rawPixels;
    std::vector<DirtyRect> dirtyRects;
    uint32_t stride;
};

// Opaque handle for an active hotplug callback registration
using HotplugHandle = void*;

} // namespace dl::sdk::qpa::core

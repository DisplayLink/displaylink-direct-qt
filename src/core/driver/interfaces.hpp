// Copyright (C) 2026 Synaptics Incorporated
#pragma once
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <driver/types.hpp>

namespace dl::sdk::qpa::core
{

class DisplayInterface
{
public:
    virtual ~DisplayInterface() = default;
    virtual std::string id() const = 0;
    virtual DisplaySize size() const = 0;
    virtual DriverStatus modes(std::vector<DisplayMode>& modes) const = 0;
    virtual DriverStatus preferredMode(DisplayMode& mode) const = 0;
    virtual DriverStatus powerOn() = 0;
    virtual DriverStatus powerOff() = 0;
    virtual DriverStatus show(const Frame& frame) = 0;
    virtual DriverStatus waitOnShow(std::chrono::milliseconds duration = std::chrono::milliseconds::max()) = 0;
    virtual void clear() = 0;
};

class DeviceInterface
{
public:
    virtual ~DeviceInterface() = default;
    virtual std::string id() const = 0;
    virtual std::vector<std::unique_ptr<DisplayInterface>> getDisplays() = 0;
};

enum class HotplugEventType
{
    DisplayArrived = 2,
    DisplayRemoved = 3,
};

// Data delivered to HotplugCallback. For DisplayArrived events, display field is populated; for DisplayRemoved events,
// displayId is populated instead
struct HotplugEvent
{
    HotplugEventType type;
    std::string displayId;
    std::shared_ptr<DisplayInterface> display;
};

using HotplugCallback = std::function<void(const HotplugEvent&)>;

class DriverInterface
{
public:
    virtual ~DriverInterface() = default;
    virtual std::vector<std::unique_ptr<DeviceInterface>> getDevices() = 0;
    virtual DriverStatus registerHotplugCallback(const HotplugCallback& callback, HotplugHandle& handle) = 0;
    virtual DriverStatus unregisterHotplugCallback(HotplugHandle handle) = 0;
};

} // namespace dl::sdk::qpa::core

// Copyright (C) 2026 Synaptics Incorporated
#include "bridge.hpp"

#include <chrono>

namespace dl::sdk::qpa::plugin::bridge
{

namespace
{

core::DriverStatus toCoreStatus(dlsdk_status status)
{
    switch (status)
    {
    case DLSDK_SUCCESS:
        return core::DriverStatus::Success;
    case DLSDK_NOT_ENOUGH_SPACE:
        return core::DriverStatus::NotEnoughSpace;
    case DLSDK_UNSUCCESSFUL_NO_DEVICE:
        return core::DriverStatus::UnsuccessfulNoDevice;
    case DLSDK_UNSUCCESSFUL_NO_MONITOR:
        return core::DriverStatus::UnsuccessfulNoMonitor;
    case DLSDK_UNSUCCESSFUL_MONITOR_OFF:
        return core::DriverStatus::UnsuccessfulMonitorOff;
    case DLSDK_INVALID_HANDLE:
        return core::DriverStatus::InvalidHandle;
    case DLSDK_INVALID_ARGS:
        return core::DriverStatus::InvalidArgs;
    case DLSDK_TIMEOUT:
        return core::DriverStatus::Timeout;
    case DLSDK_NOT_IMPLEMENTED:
        return core::DriverStatus::NotImplemented;
    case DLSDK_BUSY:
        return core::DriverStatus::Busy;
    case DLSDK_UNSUCCESSFUL:
    default:
        return core::DriverStatus::Unsuccessful;
    }
}

core::DisplaySize toCoreRect(const dlsdk_rect& rect)
{
    return {rect.width, rect.height};
}

core::DisplayMode toCoreMode(const dlsdk_display_mode& mode)
{
    return {toCoreRect(mode.resolution), mode.refreshRateHz};
}

dlsdk_dirty_rect toSdkDirtyRect(const core::DirtyRect& rect)
{
    return {.x = rect.x, .y = rect.y, .width = rect.width, .height = rect.height};
}

std::vector<dlsdk_dirty_rect> toSdkDirtyRects(const std::vector<core::DirtyRect>& rects)
{
    std::vector<dlsdk_dirty_rect> sdkRects;
    sdkRects.reserve(rects.size());
    for (const auto& rect : rects)
    {
        sdkRects.push_back(toSdkDirtyRect(rect));
    }
    return sdkRects;
}

} // namespace

Display::Display(dl::sdk::DisplayHandle display)
    : m_display(std::move(display))
{
}

Display::~Display()
{
}

std::string Display::id() const
{
    return m_display.id();
}

core::DisplaySize Display::size() const
{
    return toCoreRect(m_display.size());
}

core::DriverStatus Display::modes(std::vector<core::DisplayMode>& modes) const
{
    modes.clear();

    unsigned int count = 0;
    dlsdk_status result = m_display.modes(nullptr, &count);
    if (result != DLSDK_SUCCESS && result != DLSDK_NOT_ENOUGH_SPACE)
    {
        return toCoreStatus(result);
    }

    std::vector<dlsdk_display_mode> sdkModes(count);
    result = m_display.modes(sdkModes.data(), &count);
    if (result != DLSDK_SUCCESS)
    {
        return toCoreStatus(result);
    }

    sdkModes.resize(count);
    modes.reserve(sdkModes.size());
    for (const auto& sdkMode : sdkModes)
    {
        modes.push_back(toCoreMode(sdkMode));
    }
    return core::DriverStatus::Success;
}

core::DriverStatus Display::preferredMode(core::DisplayMode& mode) const
{
    dlsdk_display_mode sdkMode{};
    const dlsdk_status result = m_display.preferredMode(&sdkMode);
    mode = toCoreMode(sdkMode);
    return toCoreStatus(result);
}

core::DriverStatus Display::powerOn()
{
    return toCoreStatus(m_display.powerOn());
}

core::DriverStatus Display::powerOff()
{
    return toCoreStatus(m_display.powerOff());
}

core::DriverStatus Display::show(const core::Frame& frame)
{
    const auto& sdkDirtyRects = toSdkDirtyRects(frame.dirtyRects);

    const dlsdk_frame sdkFrame = {
        .size = sizeof(dlsdk_frame),
        .format = DLSDK_PIXEL_FORMAT_XRGB,
        .pixels = frame.rawPixels.data(),
        .pixelsSize = static_cast<uint32_t>(frame.rawPixels.size()),
        .stride = frame.stride,
        .dirtyRects = sdkDirtyRects.data(),
        .dirtyRectCount = static_cast<uint32_t>(sdkDirtyRects.size()),
        .staticMetadata = nullptr,
    };
    return toCoreStatus(m_display.show(sdkFrame));
}

core::DriverStatus Display::waitOnShow(std::chrono::milliseconds duration)
{
    return toCoreStatus(m_display.waitOnShow(duration));
}

void Display::clear()
{
    m_display.clear();
}

Device::Device(dl::sdk::DeviceHandle device)
    : m_device(std::move(device))
{
}

Device::~Device()
{
}

std::string Device::id() const
{
    return m_device.id();
}

std::vector<std::unique_ptr<core::DisplayInterface>> Device::getDisplays()
{
    std::vector<std::unique_ptr<core::DisplayInterface>> displays;
    auto sdkDisplays = m_device.getDisplays();
    for (auto& sdkDisp : sdkDisplays)
    {
        displays.emplace_back(std::make_unique<Display>(std::move(sdkDisp)));
    }
    return displays;
}

Driver::Driver()
{
}

Driver::~Driver()
{
}

std::vector<std::unique_ptr<core::DeviceInterface>> Driver::getDevices()
{
    std::vector<std::unique_ptr<core::DeviceInterface>> devices;
    auto sdkDevices = m_driver.getDevices();
    for (auto& sdkDev : sdkDevices)
    {
        devices.emplace_back(std::make_unique<Device>(std::move(sdkDev)));
    }
    return devices;
}

core::DriverStatus Driver::registerHotplugCallback(const core::HotplugCallback& callback, core::HotplugHandle& handle)
{
    auto sdkCallback = [callback](const dl::sdk::HotplugEventData& data)
    {
        switch (data.event)
        {
        case DLSDK_HOTPLUG_EVENT_DISPLAY_ARRIVED:
            callback(
                core::HotplugEvent{core::HotplugEventType::DisplayArrived, "", std::make_shared<Display>(data.display)}
            );
            break;
        case DLSDK_HOTPLUG_EVENT_DISPLAY_REMOVED:
            callback(core::HotplugEvent{core::HotplugEventType::DisplayRemoved, data.display_id, nullptr});
            break;
        default:
            break;
        }
    };

    dl::sdk::CallbackHandle sdkHandle = nullptr;
    const dlsdk_status result = m_driver.registerHotplugCallback(sdkCallback, &sdkHandle);
    handle = sdkHandle;
    return toCoreStatus(result);
}

core::DriverStatus Driver::unregisterHotplugCallback(core::HotplugHandle handle)
{
    return toCoreStatus(m_driver.unregisterHotplugCallback(static_cast<dl::sdk::CallbackHandle>(handle)));
}

const char* describeDriverStatus(core::DriverStatus status)
{
    switch (status)
    {
    case core::DriverStatus::Success:
        return "success";
    case core::DriverStatus::NotEnoughSpace:
        return "not enough space";
    case core::DriverStatus::UnsuccessfulNoDevice:
        return "no device available";
    case core::DriverStatus::UnsuccessfulNoMonitor:
        return "no monitor available";
    case core::DriverStatus::UnsuccessfulMonitorOff:
        return "monitor is powered off";
    case core::DriverStatus::InvalidHandle:
        return "invalid handle";
    case core::DriverStatus::InvalidArgs:
        return "invalid arguments";
    case core::DriverStatus::Timeout:
        return "operation timed out";
    case core::DriverStatus::NotImplemented:
        return "operation not implemented";
    case core::DriverStatus::Busy:
        return "resource is busy";
    case core::DriverStatus::Unsuccessful:
        return "operation failed";
    default:
        return "unknown driver status";
    }
}

} // namespace dl::sdk::qpa::plugin::bridge

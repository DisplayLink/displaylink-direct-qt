// Copyright (C) 2026 Synaptics Incorporated
#pragma once
#include <chrono>
#include <vector>

#include <dlsdk/dlsdk.h>
#include <driver/interfaces.hpp>

namespace dl::sdk::qpa::plugin::bridge
{

inline constexpr int MaxDirtyRects = DLSDK_MAX_DIRTY_RECTS;

class Display final : public core::DisplayInterface
{
public:
    explicit Display(dl::sdk::DisplayHandle display);
    ~Display() override;
    std::string id() const override;
    core::DisplaySize size() const override;
    core::DriverStatus modes(std::vector<core::DisplayMode>& modes) const override;
    core::DriverStatus preferredMode(core::DisplayMode& mode) const override;
    core::DriverStatus powerOn() override;
    core::DriverStatus powerOff() override;
    core::DriverStatus show(const core::Frame& frame) override;
    core::DriverStatus waitOnShow(std::chrono::milliseconds duration = std::chrono::milliseconds::max()) override;
    void clear() override;

private:
    dl::sdk::DisplayHandle m_display;
};

class Device final : public core::DeviceInterface
{
public:
    explicit Device(dl::sdk::DeviceHandle device);
    ~Device() override;
    std::string id() const override;
    std::vector<std::unique_ptr<core::DisplayInterface>> getDisplays() override;

private:
    dl::sdk::DeviceHandle m_device;
};

class Driver final : public core::DriverInterface
{
public:
    Driver();
    ~Driver() override;
    std::vector<std::unique_ptr<core::DeviceInterface>> getDevices() override;
    core::DriverStatus
    registerHotplugCallback(const core::HotplugCallback& callback, core::HotplugHandle& handle) override;
    core::DriverStatus unregisterHotplugCallback(core::HotplugHandle handle) override;

private:
    dl::sdk::System m_driver;
};

const char* describeDriverStatus(core::DriverStatus status);

} // namespace dl::sdk::qpa::plugin::bridge

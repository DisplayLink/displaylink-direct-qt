// Copyright (C) 2026 Synaptics Incorporated
#pragma once
#include <chrono>
#include <memory>

#include <driver/interfaces.hpp>
#include <gmock/gmock.h>

namespace dl::sdk::qpa::tests
{

class MockDisplay : public core::DisplayInterface
{
public:
    MOCK_METHOD(std::string, id, (), (const, override));
    MOCK_METHOD(core::DisplaySize, size, (), (const, override));
    MOCK_METHOD(core::DriverStatus, modes, (std::vector<core::DisplayMode>&), (const, override));
    MOCK_METHOD(core::DriverStatus, preferredMode, (core::DisplayMode&), (const, override));
    MOCK_METHOD(core::DriverStatus, powerOn, (), (override));
    MOCK_METHOD(core::DriverStatus, powerOff, (), (override));
    MOCK_METHOD(core::DriverStatus, show, (const core::Frame&), (override));
    MOCK_METHOD(core::DriverStatus, waitOnShow, (std::chrono::milliseconds), (override));
    MOCK_METHOD(void, clear, (), (override));
};

class MockDevice : public core::DeviceInterface
{
public:
    MOCK_METHOD(std::string, id, (), (const, override));
    MOCK_METHOD(std::vector<std::unique_ptr<core::DisplayInterface>>, getDisplays, (), (override));
};

class MockDriver : public core::DriverInterface
{
public:
    MOCK_METHOD(std::vector<std::unique_ptr<core::DeviceInterface>>, getDevices, (), (override));
    MOCK_METHOD(
        core::DriverStatus, registerHotplugCallback, (const core::HotplugCallback&, core::HotplugHandle&), (override)
    );
    MOCK_METHOD(core::DriverStatus, unregisterHotplugCallback, (core::HotplugHandle), (override));
};

} // namespace dl::sdk::qpa::tests

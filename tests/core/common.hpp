#pragma once
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_driver.hpp"

namespace dl::sdk::qpa::tests
{

// Typo-proofing
static auto constexpr DISPLAY_GENERIC = "DISPLAY-GENERIC";
static auto constexpr DISPLAY_1 = "DISPLAY-1";
static auto constexpr DISPLAY_2 = "DISPLAY-2";
static auto constexpr DISPLAY_3 = "DISPLAY-3";
static auto constexpr DEVICE_GENERIC = "DEVICE-GENERIC";
static auto constexpr DEVICE_1 = "DEVICE-1";
static auto constexpr DEVICE_2 = "DEVICE-2";
static auto constexpr DEVICE_3 = "DEVICE-3";

inline std::shared_ptr<::testing::NiceMock<MockDisplay>> createMockDisplay(const std::string& id = DISPLAY_GENERIC)
{
    auto display = std::make_shared<::testing::NiceMock<MockDisplay>>();
    ON_CALL(*display, id()).WillByDefault(::testing::Return(id));
    return display;
}

inline std::unique_ptr<::testing::NiceMock<MockDevice>> createMockDevice(const std::string& id = DEVICE_GENERIC)
{
    auto device = std::make_unique<::testing::NiceMock<MockDevice>>();
    ON_CALL(*device, id()).WillByDefault(::testing::Return(id));
    return device;
}

} // namespace dl::sdk::qpa::tests

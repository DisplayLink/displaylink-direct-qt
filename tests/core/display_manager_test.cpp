// Copyright (C) 2026 Synaptics Incorporated
#include <display_manager.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common.hpp"

namespace dl::sdk::qpa::tests
{

using ::testing::_;
using ::testing::Return;

class DisplayManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_mockDriver = std::make_unique<MockDriver>();
    }

    core::DisplayManager createMockDisplayManager()
    {
        auto onAdded = [](std::shared_ptr<core::DisplayInterface>) {};
        auto onRemoved = [](const std::string&) {};
        return core::DisplayManager(m_mockDriver.get(), onAdded, onRemoved);
    }

    // If expectRegister is true, the test will expect a call to registerHotplugCallback; otherwise it will allow the
    // call without expecting it
    void createHotplugCallback(core::HotplugCallback& hotplugCallback, bool expectRegister)
    {
        if (expectRegister)
        {
            EXPECT_CALL(*m_mockDriver, registerHotplugCallback(_, _))
                .WillOnce(
                    [&](const core::HotplugCallback& callback, core::HotplugHandle& handle)
                    {
                        hotplugCallback = callback;
                        handle = reinterpret_cast<core::HotplugHandle>(1);
                        return core::DriverStatus::Success;
                    }
                );
        }
        else
        {
            ON_CALL(*m_mockDriver, registerHotplugCallback(_, _))
                .WillByDefault(
                    [&](const core::HotplugCallback& callback, core::HotplugHandle& handle)
                    {
                        hotplugCallback = callback;
                        handle = reinterpret_cast<core::HotplugHandle>(1);
                        return core::DriverStatus::Success;
                    }
                );
        }
    }

    std::unique_ptr<MockDriver> m_mockDriver;
};

TEST_F(DisplayManagerTest, InitializeSucceeds)
{
    EXPECT_CALL(*m_mockDriver, registerHotplugCallback(_, _)).WillOnce(Return(core::DriverStatus::Success));
    auto manager = createMockDisplayManager();
    bool result = manager.initialize();
    EXPECT_TRUE(result);
}

TEST_F(DisplayManagerTest, InitializeFails)
{
    EXPECT_CALL(*m_mockDriver, registerHotplugCallback(_, _)).WillOnce(Return(core::DriverStatus::Unsuccessful));
    auto manager = createMockDisplayManager();
    bool result = manager.initialize();
    EXPECT_FALSE(result);
}

TEST_F(DisplayManagerTest, InitializeIsIdempotent)
{
    EXPECT_CALL(*m_mockDriver, registerHotplugCallback(_, _)).WillRepeatedly(Return(core::DriverStatus::Success));
    auto manager = createMockDisplayManager();

    EXPECT_TRUE(manager.initialize());
    EXPECT_TRUE(manager.initialize());
}

TEST_F(DisplayManagerTest, DisplayAttachAndDetachInvokeCallbacks)
{
    core::HotplugCallback hotplugCallback;
    createHotplugCallback(hotplugCallback, true);

    std::string addedDisplayId;
    std::string removedDisplayId;

    auto onDisplayAdded = [&](std::shared_ptr<core::DisplayInterface> display) { addedDisplayId = display->id(); };
    auto onDisplayRemoved = [&](const std::string& displayId) { removedDisplayId = displayId; };

    auto manager = core::DisplayManager(m_mockDriver.get(), onDisplayAdded, onDisplayRemoved);
    manager.initialize();

    hotplugCallback(core::HotplugEvent{core::HotplugEventType::DisplayArrived, DISPLAY_1, createMockDisplay(DISPLAY_1)}
    );
    EXPECT_EQ(addedDisplayId, DISPLAY_1);

    hotplugCallback(core::HotplugEvent{core::HotplugEventType::DisplayRemoved, DISPLAY_1, nullptr});
    EXPECT_EQ(removedDisplayId, DISPLAY_1);
}

TEST_F(DisplayManagerTest, DuplicateDisplayAttachIsNoOp)
{
    core::HotplugCallback hotplugCallback;
    createHotplugCallback(hotplugCallback, true);

    int addCount = 0;

    auto onDisplayAdded = [&](std::shared_ptr<core::DisplayInterface>) { addCount++; };
    auto onDisplayRemoved = [](const std::string&) {};

    auto manager = core::DisplayManager(m_mockDriver.get(), onDisplayAdded, onDisplayRemoved);
    manager.initialize();

    hotplugCallback(core::HotplugEvent{core::HotplugEventType::DisplayArrived, DISPLAY_1, createMockDisplay(DISPLAY_1)}
    );
    EXPECT_EQ(addCount, 1);

    hotplugCallback(core::HotplugEvent{core::HotplugEventType::DisplayArrived, DISPLAY_1, createMockDisplay(DISPLAY_1)}
    );
    EXPECT_EQ(addCount, 1);
}

TEST_F(DisplayManagerTest, NonExistentDisplayDetachIsNoOp)
{
    core::HotplugCallback hotplugCallback;
    createHotplugCallback(hotplugCallback, true);

    int removeCount = 0;

    auto onDisplayAdded = [](std::shared_ptr<core::DisplayInterface>) {};
    auto onDisplayRemoved = [&](const std::string&) { removeCount++; };

    auto manager = core::DisplayManager(m_mockDriver.get(), onDisplayAdded, onDisplayRemoved);
    manager.initialize();

    hotplugCallback(core::HotplugEvent{core::HotplugEventType::DisplayRemoved, DISPLAY_1, nullptr});
    EXPECT_EQ(removeCount, 0);
}

TEST_F(DisplayManagerTest, DisplayArrivedWithNullDisplayIsIgnored)
{
    core::HotplugCallback hotplugCallback;
    createHotplugCallback(hotplugCallback, true);

    int addCount = 0;
    auto onDisplayAdded = [&](std::shared_ptr<core::DisplayInterface>) { addCount++; };
    auto onDisplayRemoved = [](const std::string&) {};
    auto manager = core::DisplayManager(m_mockDriver.get(), onDisplayAdded, onDisplayRemoved);
    manager.initialize();

    hotplugCallback(core::HotplugEvent{core::HotplugEventType::DisplayArrived, DISPLAY_1, nullptr});
    EXPECT_EQ(addCount, 0);
}

TEST_F(DisplayManagerTest, DestructorUnregistersCallback)
{
    core::HotplugCallback hotplugCallback;
    createHotplugCallback(hotplugCallback, true);

    EXPECT_CALL(*m_mockDriver, unregisterHotplugCallback(reinterpret_cast<core::HotplugHandle>(1))).Times(1);
    {
        auto manager = createMockDisplayManager();
        manager.initialize();
    }
}

} // namespace dl::sdk::qpa::tests

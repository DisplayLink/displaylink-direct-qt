// Copyright (C) 2026 Synaptics Incorporated
#include "display_manager.hpp"

namespace dl::sdk::qpa::core
{

DisplayManager::DisplayManager(
    DriverInterface* driver, DisplayAddedCallback onDisplayAddedCb, DisplayRemovedCallback onDisplayRemovedCb
)
    : m_driver(driver)
    , m_onDisplayAdded(std::move(onDisplayAddedCb))
    , m_onDisplayRemoved(std::move(onDisplayRemovedCb))
{
}

DisplayManager::~DisplayManager()
{
    if (m_hotplugCbHandle)
    {
        // Blocks until all callbacks finish executing
        m_driver->unregisterHotplugCallback(m_hotplugCbHandle);
    }
}

bool DisplayManager::initialize()
{
    if (isInitialized())
    {
        return true;
    }

    auto result = m_driver->registerHotplugCallback(
        [this](const HotplugEvent& event)
        {
            switch (event.type)
            {
            case HotplugEventType::DisplayArrived:
                if (event.display)
                {
                    handleDisplayHotplugAttach(event.display);
                }
                break;
            case HotplugEventType::DisplayRemoved:
                handleDisplayHotplugDetach(event.displayId);
                break;
            }
        },
        m_hotplugCbHandle
    );

    if (result != DriverStatus::Success)
    {
        m_hotplugCbHandle = nullptr;
        return false;
    }
    return true;
}

void DisplayManager::handleDisplayHotplugAttach(std::shared_ptr<DisplayInterface> display)
{
    std::shared_ptr<DisplayInterface> storedDisp;
    {
        std::scoped_lock lock(m_displaysMutex);

        if (m_displays.contains(display->id()))
        {
            // If a display arrives twice, that's a bug with DLSDK itself
            return;
        }

        auto [it, inserted] = m_displays.emplace(display->id(), std::move(display));
        if (inserted)
        {
            storedDisp = it->second;
        }
    }

    if (storedDisp)
    {
        m_onDisplayAdded(storedDisp);
    }
}

void DisplayManager::handleDisplayHotplugDetach(const std::string& displayId)
{
    {
        std::scoped_lock lock(m_displaysMutex);
        auto it = m_displays.find(displayId);
        if (it == m_displays.end())
        {
            // If a display is removed that was never added, that's a bug with DLSDK itself
            return;
        }
        m_displays.erase(it);
    }

    m_onDisplayRemoved(displayId);
}

bool DisplayManager::isInitialized() const
{
    return m_hotplugCbHandle != nullptr;
}

} // namespace dl::sdk::qpa::core

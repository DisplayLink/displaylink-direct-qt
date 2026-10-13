// Copyright (C) 2026 Synaptics Incorporated
#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <driver/interfaces.hpp>

namespace dl::sdk::qpa::core
{

class DisplayManager final
{
public:
    using DisplayAddedCallback = std::function<void(std::shared_ptr<DisplayInterface>)>;
    using DisplayRemovedCallback = std::function<void(const std::string&)>;

    DisplayManager(
        DriverInterface* driver, DisplayAddedCallback onDisplayAddedCb, DisplayRemovedCallback onDisplayRemovedCb
    );
    ~DisplayManager();

    bool initialize();

private:
    void handleDisplayHotplugAttach(std::shared_ptr<DisplayInterface> display);
    void handleDisplayHotplugDetach(const std::string& displayId);
    bool isInitialized() const;

    DriverInterface* m_driver;
    DisplayAddedCallback m_onDisplayAdded;
    DisplayRemovedCallback m_onDisplayRemoved;

    HotplugHandle m_hotplugCbHandle = nullptr;
    std::unordered_map<std::string, std::shared_ptr<DisplayInterface>> m_displays;
    std::mutex m_displaysMutex;
};

} // namespace dl::sdk::qpa::core

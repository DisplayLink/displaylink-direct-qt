// Copyright (C) 2026 Synaptics Incorporated
#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <string>

#include <driver/interfaces.hpp>

namespace dl::sdk::qpa::core
{

// Manages a single active display and a queue of candidate displays
class SingleDisplayController final
{
public:
    using AttachCallback = std::function<bool(std::shared_ptr<DisplayInterface>)>;
    using DetachCallback = std::function<void(const std::string&)>;

    SingleDisplayController(AttachCallback attachDisplay, DetachCallback detachDisplay);

    void onDisplayAdded(std::shared_ptr<DisplayInterface> display);
    void onDisplayRemoved(const std::string& displayId);

    const std::string& activeDisplayId() const;
    size_t candidateCount() const;

private:
    struct DisplayCandidate
    {
        std::string id;
        std::shared_ptr<DisplayInterface> display;
    };

    bool attachNextDisplay();

    AttachCallback m_attachDisplay;
    DetachCallback m_detachDisplay;

    std::string m_activeDisplayId;
    std::deque<DisplayCandidate> m_candidateDisplays;
};

} // namespace dl::sdk::qpa::core

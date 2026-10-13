// Copyright (C) 2026 Synaptics Incorporated
#include "single_display_controller.hpp"

#include <algorithm>
#include <utility>

namespace dl::sdk::qpa::core
{

SingleDisplayController::SingleDisplayController(AttachCallback attachDisplay, DetachCallback detachDisplay)
    : m_attachDisplay(std::move(attachDisplay))
    , m_detachDisplay(std::move(detachDisplay))
{
}

void SingleDisplayController::onDisplayAdded(std::shared_ptr<DisplayInterface> display)
{
    if (!display)
    {
        return;
    }

    const std::string displayId = display->id();
    if (displayId == m_activeDisplayId)
    {
        return;
    }

    const auto existingCandidate = std::find_if(
        m_candidateDisplays.begin(),
        m_candidateDisplays.end(),
        [&displayId](const DisplayCandidate& candidate) { return candidate.id == displayId; }
    );
    if (existingCandidate != m_candidateDisplays.end())
    {
        return;
    }

    if (m_activeDisplayId.empty() && m_attachDisplay(display))
    {
        m_activeDisplayId = displayId;
        return;
    }

    m_candidateDisplays.push_back({displayId, std::move(display)});
}

void SingleDisplayController::onDisplayRemoved(const std::string& displayId)
{
    if (displayId == m_activeDisplayId)
    {
        m_detachDisplay(displayId);
        m_activeDisplayId.clear();
        attachNextDisplay();
        return;
    }

    const auto candidate = std::find_if(
        m_candidateDisplays.begin(),
        m_candidateDisplays.end(),
        [&displayId](const DisplayCandidate& item) { return item.id == displayId; }
    );
    if (candidate != m_candidateDisplays.end())
    {
        m_candidateDisplays.erase(candidate);
    }
}

const std::string& SingleDisplayController::activeDisplayId() const
{
    return m_activeDisplayId;
}

size_t SingleDisplayController::candidateCount() const
{
    return m_candidateDisplays.size();
}

bool SingleDisplayController::attachNextDisplay()
{
    const size_t candidateCount = m_candidateDisplays.size();
    for (size_t i = 0; i < candidateCount; ++i)
    {
        DisplayCandidate candidate = std::move(m_candidateDisplays.front());
        m_candidateDisplays.pop_front();
        if (m_attachDisplay(candidate.display))
        {
            m_activeDisplayId = std::move(candidate.id);
            return true;
        }

        // Keep failed candidate for future failover
        m_candidateDisplays.push_back(std::move(candidate));
    }
    return false;
}

} // namespace dl::sdk::qpa::core

// Copyright (C) 2026 Synaptics Incorporated
#include "screen.hpp"

#include <QLoggingCategory>
#include <QPainter>
#include <QRegion>
#include <QtFbSupport/private/qfbwindow_p.h>

#include <span>

#include "bridge.hpp"
#include "dirty_rects.hpp"
#include "logging.hpp"

namespace dl::sdk::qpa::plugin
{

Screen::Screen(ScreenConfig config)
    : m_config(std::move(config))
{
    mDepth = 32;
    mFormat = QImage::Format_ARGB32;
}

Screen::~Screen()
{
    if (m_display)
    {
        m_display->powerOff();
    }
    delete mCursor;
    mCursor = nullptr;
}

bool Screen::initialize()
{
    if (m_config.drawCursor)
    {
        logInfo(g_lcDraw) << "Software cursor drawing is enabled";
        mCursor = new QFbCursor(this);
    }

    mGeometry = QRect(0, 0, InitialWidth, InitialHeight);
    QFbScreen::initializeCompositor();
    if (mScreenImage.isNull())
    {
        logCritical(g_lcDraw) << "Failed to allocate buffer for screen framebuffer";
        return false;
    }

    logInfo(g_lcDraw) << QString("Created detached screen with resolution %1x%2 and origin at %3,%4")
                             .arg(mGeometry.width())
                             .arg(mGeometry.height())
                             .arg(mGeometry.x())
                             .arg(mGeometry.y());
    return true;
}

bool Screen::attachDisplay(std::shared_ptr<core::DisplayInterface> display)
{
    if (!display || m_display)
    {
        return false;
    }
    const auto displayId = display->id();

    const auto powerOnResult = display->powerOn();
    if (powerOnResult != core::DriverStatus::Success)
    {
        logCritical(g_lcDisplay) << QString("Failed to power on display %1 (status: %2, code: %3)")
                                        .arg(displayId.c_str())
                                        .arg(bridge::describeDriverStatus(powerOnResult))
                                        .arg(static_cast<int>(powerOnResult));
        return false;
    }

    const auto displaySize = display->size();
    const size_t width = displaySize.width;
    const size_t height = displaySize.height;
    if (width == 0 || width > MaxWidth || height == 0 || height > MaxHeight)
    {
        display->powerOff();
        logCritical(g_lcDisplay) << QString("Failed to set up display %1 due to invalid resolution %2x%3")
                                        .arg(displayId.c_str())
                                        .arg(width)
                                        .arg(height);
        return false;
    }

    const QRect previousGeometry = mGeometry;
    const QRect displayGeometry(0, 0, static_cast<int>(width), static_cast<int>(height));
    if (displayGeometry != mGeometry)
    {
        setGeometry(displayGeometry);
        if (mScreenImage.isNull())
        {
            setGeometry(previousGeometry);
            display->powerOff();
            logCritical(g_lcDisplay) << QString("Failed to allocate framebuffer for display %1").arg(displayId.c_str());
            return false;
        }
    }

    m_display = std::move(display);
    m_displayId = displayId;
    setDirty(mGeometry);
    logInfo(g_lcDisplay) << QString("Attached display %1 to screen with resolution %2x%3")
                                .arg(m_displayId.c_str())
                                .arg(mGeometry.width())
                                .arg(mGeometry.height());
    return true;
}

bool Screen::detachDisplay(const std::string& displayId)
{
    if (!m_display || m_displayId != displayId)
    {
        return false;
    }

    m_display->powerOff();
    m_display.reset();
    m_displayId.clear();
    logInfo(g_lcDisplay) << QString("Detached display %1 from screen").arg(displayId.c_str());
    return true;
}

bool Screen::hasDisplay() const
{
    return static_cast<bool>(m_display);
}

const std::string& Screen::displayId() const
{
    return m_displayId;
}

QDpi Screen::logicalDpi() const
{
    return {m_config.logicalDpi, m_config.logicalDpi};
}

QRegion Screen::doRedraw()
{
    if (!m_display)
    {
        logWarning(g_lcDraw) << "Attempted a redraw without an attached display";
        return {};
    }

    auto* buffer = frameBuffer();
    if (!buffer)
    {
        logWarning(g_lcDraw) << "Attempted a redraw without a valid framebuffer";
        return {};
    }

    auto dirtyRects = QFbScreen::doRedraw();
    const auto fullRect = mScreenImage.rect();
    if (dirtyRects.isEmpty())
    {
        return dirtyRects;
    }

    const auto submissionRegion = m_fullSubmissionRequired ? QRegion(fullRect) : dirtyRects.intersected(fullRect);
    const auto submissionRects = toDirtyRects(submissionRegion, bridge::MaxDirtyRects);
    logSubmissionDetails(submissionRects, submissionRegion.rectCount());

    const core::Frame frame = {
        .rawPixels = std::span{static_cast<const uint8_t*>(buffer), frameBufferSize()},
        .dirtyRects = submissionRects,
        .stride = 0
    };
    const auto showResult = m_display->show(frame);

    if (showResult != core::DriverStatus::Success)
    {
        logWarning(g_lcDraw) << QString("Failed to submit frame to display %1, next submission will contain full frame "
                                        "(status: %2, code: %3)")
                                    .arg(m_displayId.c_str())
                                    .arg(bridge::describeDriverStatus(showResult))
                                    .arg(static_cast<int>(showResult));
        m_fullSubmissionRequired = true;
        return dirtyRects;
    }

    m_fullSubmissionRequired = false;

    const auto waitResult = m_display->waitOnShow(std::chrono::milliseconds(100));
    if (waitResult != core::DriverStatus::Success)
    {
        logWarning(g_lcDraw) << QString("Failed to wait for frame on display %1 (status: %2, code: %3)")
                                    .arg(m_displayId.c_str())
                                    .arg(bridge::describeDriverStatus(waitResult))
                                    .arg(static_cast<int>(waitResult));
    }

    return dirtyRects;
}

const void* Screen::frameBuffer() const
{
    if (mScreenImage.isNull())
    {
        return nullptr;
    }
    return mScreenImage.bits();
}

size_t Screen::frameBufferSize() const
{
    auto dim = m_display->size();
    // Assuming ARGB
    return dim.width * dim.height * 4;
}

void Screen::logSubmissionDetails(const std::vector<core::DirtyRect>& submissionRects, const int sourceRectCount) const
{
    if (!g_lcDraw().isDebugEnabled())
    {
        return;
    }
    if (submissionRects.empty())
    {
        return;
    }

    const auto fullRect = mScreenImage.rect();
    const auto fullArea = static_cast<size_t>(fullRect.width()) * static_cast<size_t>(fullRect.height());
    size_t submissionArea = 0;

    if (fullArea == 0)
    {
        return;
    }

    // First construct the details for each dirty rect to sum up coverage in same pass
    QString rectMessage;
    int i = 1;
    for (const auto& rect : submissionRects)
    {
        const auto area = static_cast<size_t>(rect.width) * static_cast<size_t>(rect.height);
        submissionArea += area;
        const double coverage = (static_cast<double>(area) / static_cast<double>(fullArea)) * 100.0;

        rectMessage += QString("    [%1] x=%2 y=%3 width=%4 height=%5 area=%6px coverage=%7%\n")
                           .arg(i)
                           .arg(rect.x)
                           .arg(rect.y)
                           .arg(rect.width)
                           .arg(rect.height)
                           .arg(area)
                           .arg(coverage, 0, 'f', 2);
        i++;
    }

    // Remove last newline
    if (!rectMessage.isEmpty())
    {
        rectMessage.chop(1);
    }

    const auto totalCoverage = (static_cast<double>(submissionArea) / static_cast<double>(fullArea)) * 100.0;
    QString message = QString("Submitting frame to display %1 with %2 dirty rects (collapsed=%3, coverage=%4%)\n")
                          .arg(m_displayId.c_str())
                          .arg(submissionRects.size())
                          .arg(sourceRectCount > bridge::MaxDirtyRects ? "true" : "false")
                          .arg(totalCoverage, 0, 'f', 2);
    logDebug(g_lcDraw) << message + rectMessage;
}

} // namespace dl::sdk::qpa::plugin

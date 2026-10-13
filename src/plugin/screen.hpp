// Copyright (C) 2026 Synaptics Incorporated
#pragma once
#include <QtFbSupport/private/qfbscreen_p.h>
#include <qpa/qwindowsysteminterface.h>

#include <memory>
#include <string>
#include <vector>

#include <driver/interfaces.hpp>

class QImage;
class QPainter;
class QRegion;

namespace dl::sdk::qpa::plugin
{

struct ScreenConfig
{
    bool drawCursor = false;
    qreal logicalDpi = 96.0;
};

class Screen : public QFbScreen
{
    Q_OBJECT
public:
    explicit Screen(ScreenConfig config = {});
    ~Screen();

    bool initialize() override;
    bool attachDisplay(std::shared_ptr<core::DisplayInterface> display);
    bool detachDisplay(const std::string& displayId);
    bool hasDisplay() const;
    const std::string& displayId() const;
    QDpi logicalDpi() const override;
    QRegion doRedraw() override;

private:
    // Only valid after a successful call to initialize
    const void* frameBuffer() const;
    size_t frameBufferSize() const;
    void logSubmissionDetails(const std::vector<core::DirtyRect>& submissionRects, const int sourceRectCount) const;

    std::shared_ptr<core::DisplayInterface> m_display;
    std::string m_displayId;
    ScreenConfig m_config;
    bool m_fullSubmissionRequired = false;

    static constexpr int InitialWidth = 640;
    static constexpr int InitialHeight = 480;
    static constexpr size_t MaxWidth = 16384;
    static constexpr size_t MaxHeight = 16384;
};

class QPlatformScreenDeleter
{
public:
    void operator()(QPlatformScreen* screen)
    {
        if (!screen)
        {
            return;
        }
        // Qt takes ownership of deletion while handling screen removal
        QWindowSystemInterface::handleScreenRemoved(screen);
    }
};

using QPlatformScreenUniquePtr = std::unique_ptr<QPlatformScreen, QPlatformScreenDeleter>;

} // namespace dl::sdk::qpa::plugin

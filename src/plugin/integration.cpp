// Copyright (C) 2026 Synaptics Incorporated
#include "integration.hpp"

#include <QGuiApplication>
#include <QtFbSupport/private/qfbbackingstore_p.h>
#include <QtFbSupport/private/qfbvthandler_p.h>
#include <QtFbSupport/private/qfbwindow_p.h>
#include <QtGui/private/qgenericunixeventdispatcher_p.h>
#include <QtGui/private/qgenericunixfontdatabase_p.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatformservices.h>
#include <qpa/qwindowsysteminterface.h>

#include <cmath>

#if QT_CONFIG(libinput)
    #include <QtInputSupport/private/qlibinputhandler_p.h>
#endif // QT_CONFIG(libinput)

#if QT_CONFIG(evdev)
    #include <QtInputSupport/private/qevdevkeyboardmanager_p.h>
    #include <QtInputSupport/private/qevdevmousemanager_p.h>
    #include <QtInputSupport/private/qevdevtouchmanager_p.h>
#endif // QT_CONFIG(evdev)

#include "bridge.hpp"
#include "logging.hpp"

namespace dl::sdk::qpa::plugin
{

namespace
{

ScreenConfig parseScreenConfig(const QStringList& args)
{
    ScreenConfig config;
    config.drawCursor = args.contains(QLatin1String("cursor"), Qt::CaseInsensitive);

    for (const auto& arg : args)
    {
        if (!arg.startsWith(QLatin1String("dpi="), Qt::CaseInsensitive))
        {
            continue;
        }

        bool valid = false;
        const qreal dpi = arg.sliced(4).toDouble(&valid);
        if (!valid || !std::isfinite(dpi) || dpi <= 0.0)
        {
            logWarning(g_lcPlugin) << QString("Ignoring invalid DPI platform argument '%1'").arg(arg);
            continue;
        }

        config.logicalDpi = dpi;
    }
    return config;
}

} // namespace

Integration::Integration(const QStringList& args)
    : m_screenConfig(parseScreenConfig(args))
    , m_driver(std::make_unique<bridge::Driver>())
{
    createDisplayController();

    // DLSDK invokes hotplug callbacks on its own internal thread, however the Qt screen state belongs to this object's
    // thread. Therefore we place it in the Qt event queue.
    auto onDisplayAdded = [this](std::shared_ptr<core::DisplayInterface> display)
    { QMetaObject::invokeMethod(this, [this, display]() { handleDisplayAdded(display); }, Qt::QueuedConnection); };

    auto onDisplayRemoved = [this](const std::string& displayId)
    {
        QMetaObject::invokeMethod(this, [this, displayId]() { handleDisplayRemoved(displayId); }, Qt::QueuedConnection);
    };

    m_displayManager = std::make_unique<core::DisplayManager>(m_driver.get(), onDisplayAdded, onDisplayRemoved);
}

Integration::Integration(
    const QStringList& args,
    std::unique_ptr<core::DriverInterface> driver,
    std::unique_ptr<core::DisplayManager> displayManager
)
    : m_screenConfig(parseScreenConfig(args))
    , m_driver(std::move(driver))
    , m_displayManager(std::move(displayManager))
{
    createDisplayController();
}

Integration::~Integration()
{
}

void Integration::initialize()
{
    logInfo(g_lcPlugin) << "Initializing platform integration";

    if (!createPersistentScreen())
    {
        logCritical(g_lcPlugin) << "Failed to create a persistent screen";
        std::abort();
    }

    if (!m_displayManager->initialize())
    {
        logCritical(g_lcPlugin) << "Failed to create display hotplug event handler";
        std::abort();
    }

    m_fontDatabase = std::make_unique<QGenericUnixFontDatabase>();
    m_services = std::make_unique<QPlatformServices>();
    m_inputContext.reset(QPlatformInputContextFactory::create());
    m_vtHandler = std::make_unique<QFbVtHandler>();
    createInputHandlers();
}

void Integration::destroy()
{
    m_displayManager.reset();
    m_displayController.reset();
    m_screen.reset();
}

bool Integration::hasCapability(Capability cap) const
{
    switch (cap)
    {
    case ThreadedPixmaps:
        return true;
    case WindowManagement:
    case RhiBasedRendering:
        return false;
    default:
        return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformWindow* Integration::createPlatformWindow(QWindow* window) const
{
    return new QFbWindow(window);
}

QPlatformBackingStore* Integration::createPlatformBackingStore(QWindow* window) const
{
    return new QFbBackingStore(window);
}

QAbstractEventDispatcher* Integration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

QPlatformFontDatabase* Integration::fontDatabase() const
{
    return m_fontDatabase.get();
}

QPlatformServices* Integration::services() const
{
    return m_services.get();
}

QPlatformInputContext* Integration::inputContext() const
{
    return m_inputContext.get();
}

QPlatformNativeInterface* Integration::nativeInterface() const
{
    return const_cast<Integration*>(this);
}

QFunctionPointer Integration::platformFunction(const QByteArray& function) const
{
    Q_UNUSED(function);
    return nullptr;
}

#if QT_CONFIG(evdev)

void Integration::loadKeymap(const QString& filename)
{
    if (!m_keyboardMgr)
    {
        logWarning(g_lcInput) << "Cannot load keymap because no evdev keyboard manager is active";
        return;
    }
    m_keyboardMgr->loadKeymap(filename);
}

void Integration::switchLang()
{
    if (!m_keyboardMgr)
    {
        logWarning(g_lcInput) << "Cannot switch language because no evdev keyboard manager is active";
        return;
    }
    m_keyboardMgr->switchLang();
}

#endif // QT_CONFIG(evdev)

void Integration::createInputHandlers()
{
#if QT_CONFIG(libinput)
    logInfo(g_lcInput) << "Using libinput as input handling backend";
    m_inputHandler = std::make_unique<QLibInputHandler>(QLatin1String("libinput"), QString());
    return;
#endif // QT_CONFIG(libinput)

#if QT_CONFIG(evdev)
    logInfo(g_lcInput) << "Using evdev as input handling backend";
    m_keyboardMgr = std::make_unique<QEvdevKeyboardManager>(QLatin1String("EvdevKeyboard"), QString(), this);
    m_mouseMgr = std::make_unique<QEvdevMouseManager>(QLatin1String("EvdevMouse"), QString(), this);
    m_touchMgr = std::make_unique<QEvdevTouchManager>(QLatin1String("EvdevTouch"), QString(), this);
#endif // QT_CONFIG(evdev)
}

void Integration::createDisplayController()
{
    auto attachDisplay = [this](std::shared_ptr<core::DisplayInterface> display)
    { return screen() && screen()->attachDisplay(std::move(display)); };
    auto detachDisplay = [this](const std::string& displayId)
    {
        if (screen())
        {
            screen()->detachDisplay(displayId);
        }
    };
    m_displayController =
        std::make_unique<core::SingleDisplayController>(std::move(attachDisplay), std::move(detachDisplay));
}

bool Integration::createPersistentScreen()
{
    auto persistentScreen = std::make_unique<Screen>(m_screenConfig);
    if (!persistentScreen->initialize())
    {
        return false;
    }

    Screen* const screenPtr = persistentScreen.release();
    m_screen.reset(screenPtr);
    QWindowSystemInterface::handleScreenAdded(screenPtr, true);
    return true;
}

void Integration::handleDisplayAdded(std::shared_ptr<core::DisplayInterface> display)
{
    if (!display || !m_displayController)
    {
        return;
    }

    const std::string displayId = display->id();
    const std::string previousActiveDisplay = m_displayController->activeDisplayId();
    const size_t previousCandidateCount = m_displayController->candidateCount();
    m_displayController->onDisplayAdded(std::move(display));

    if (previousActiveDisplay != m_displayController->activeDisplayId())
    {
        logInfo(g_lcDisplay) << QString("Selected display %1 as active").arg(displayId.c_str());
    }
    else if (previousCandidateCount != m_displayController->candidateCount())
    {
        logInfo(g_lcDisplay) << QString("Queued display %1 as a candidate").arg(displayId.c_str());
    }
}

void Integration::handleDisplayRemoved(const std::string& displayId)
{
    if (!m_displayController)
    {
        return;
    }

    const std::string previousActiveDisplay = m_displayController->activeDisplayId();
    m_displayController->onDisplayRemoved(displayId);
    const std::string& activeDisplay = m_displayController->activeDisplayId();
    if (previousActiveDisplay == displayId && !activeDisplay.empty())
    {
        logInfo(g_lcDisplay) << QString("Failed over to display %1").arg(activeDisplay.c_str());
    }
}

Screen* Integration::screen() const
{
    return static_cast<Screen*>(m_screen.get());
}

} // namespace dl::sdk::qpa::plugin

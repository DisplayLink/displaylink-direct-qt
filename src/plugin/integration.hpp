// Copyright (C) 2026 Synaptics Incorporated
#pragma once
#include <QtFbSupport/private/qfbscreen_p.h>
#include <QtFbSupport/private/qfbvthandler_p.h>
#include <QtGui/private/qkeymapper_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include <cstdlib>
#include <memory>
#include <string>

#include <display_manager.hpp>
#include <single_display_controller.hpp>

#include "screen.hpp"

#if QT_CONFIG(libinput)
class QLibInputHandler;
#endif // QT_CONFIG(libinput)

#if QT_CONFIG(evdev)
class QEvdevKeyboardManager;
class QEvdevMouseManager;
class QEvdevTouchManager;
#endif // QT_CONFIG(evdev)

namespace dl::sdk::qpa::plugin
{

class Integration : public QPlatformIntegration,
                    public QPlatformNativeInterface,
#if QT_CONFIG(evdev)
                    public QNativeInterface::Private::QEvdevKeyMapper
#endif
{
public:
    Integration(const QStringList& params);
    Integration(
        const QStringList& params,
        std::unique_ptr<core::DriverInterface> driver,
        std::unique_ptr<core::DisplayManager> displayManager
    );

    ~Integration();

    void initialize() override;
    void destroy() override;
    bool hasCapability(Capability cap) const override;

    QPlatformWindow* createPlatformWindow(QWindow* window) const override;
    QPlatformBackingStore* createPlatformBackingStore(QWindow* window) const override;
    QAbstractEventDispatcher* createEventDispatcher() const override;

    QPlatformFontDatabase* fontDatabase() const override;
    QPlatformServices* services() const override;
    QPlatformInputContext* inputContext() const override;
    QPlatformNativeInterface* nativeInterface() const override;
    QFunctionPointer platformFunction(const QByteArray& function) const override;

#if QT_CONFIG(evdev)
    void loadKeymap(const QString& filename) override;
    void switchLang() override;
#endif

private:
    void createInputHandlers();
    void createDisplayController();

    bool createPersistentScreen();
    void handleDisplayAdded(std::shared_ptr<core::DisplayInterface> display);
    void handleDisplayRemoved(const std::string& displayId);
    Screen* screen() const;

    ScreenConfig m_screenConfig;

    std::unique_ptr<QPlatformInputContext> m_inputContext;
    std::unique_ptr<QPlatformFontDatabase> m_fontDatabase;
    std::unique_ptr<QPlatformServices> m_services;
    std::unique_ptr<QFbVtHandler> m_vtHandler;

    std::unique_ptr<core::DriverInterface> m_driver;
    std::unique_ptr<core::DisplayManager> m_displayManager;
    QPlatformScreenUniquePtr m_screen;
    std::unique_ptr<core::SingleDisplayController> m_displayController;

#if QT_CONFIG(libinput)
    std::unique_ptr<QLibInputHandler> m_inputHandler;
#endif // QT_CONFIG(libinput)

#if QT_CONFIG(evdev)
    std::unique_ptr<QEvdevKeyboardManager> m_keyboardMgr;
    std::unique_ptr<QEvdevMouseManager> m_mouseMgr;
    std::unique_ptr<QEvdevTouchManager> m_touchMgr;
#endif // QT_CONFIG(evdev)
};

} // namespace dl::sdk::qpa::plugin

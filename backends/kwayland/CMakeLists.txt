
set(wayland_SRCS
    waylandbackend.cpp
    waylandconfig.cpp
    waylandoutputdevice.cpp
    waylandoutputmanagement.cpp
    waylandoutputdevicemode.cpp
    waylandscreen.cpp
    ../utils.cpp
)
qt_add_dbus_interface(wayland_SRCS org.kde.KWin.TabletModeManager.xml tabletmodemanager_interface)

ecm_qt_declare_logging_category(wayland_SRCS
                                HEADER kscreen_kwayland_logging.h
                                IDENTIFIER KSCREEN_WAYLAND
                                CATEGORY_NAME kscreen.kwayland
)

ecm_add_qtwayland_client_protocol(wayland_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/kde-output-device-v2.xml
    BASENAME kde-output-device-v2
)

ecm_add_qtwayland_client_protocol(wayland_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/kde-output-management-v2.xml
    BASENAME kde-output-management-v2
)


add_library(KSC_KWayland MODULE ${wayland_SRCS})

set_target_properties(KSC_KWayland PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf5/kscreen")
set_target_properties(KSC_KWayland PROPERTIES PREFIX "")
target_link_libraries(KSC_KWayland Qt::Core
                                  Qt::DBus
                                  Qt::Gui
                                  KF5::Screen
                                  KF5::WaylandClient
                                  Wayland::Client
)

install(TARGETS KSC_KWayland DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kscreen/)

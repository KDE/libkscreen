
set(wayland_SRCS
    waylandbackend.cpp waylandbackend.h
    waylandconfig.cpp waylandconfig.h
    waylandoutputdevice.cpp waylandoutputdevice.h
    waylandoutputmanagement.cpp waylandoutputmanagement.h
    waylandoutputdevicemode.cpp waylandoutputdevicemode.h
    waylandscreen.cpp waylandscreen.h
    ../utils.cpp ../utils.h
)
qt_add_dbus_interface(wayland_SRCS org.kde.KWin.TabletModeManager.xml tabletmodemanager_interface)

ecm_qt_declare_logging_category(wayland_SRCS
                                HEADER kscreen_kwayland_logging.h
                                IDENTIFIER KSCREEN_WAYLAND
                                CATEGORY_NAME kscreen.kwayland
				DESCRIPTION "kscreen wayland"
				EXPORT KSCREEN
)

add_library(KSC_KWayland MODULE)

if (Qt6_VERSION VERSION_GREATER_EQUAL "6.8.0")
    set(private_code_option  "PRIVATE_CODE")
endif()
qt6_generate_wayland_protocol_client_sources(KSC_KWayland
    FILES
        ${PLASMA_WAYLAND_PROTOCOLS_DIR}/kde-output-device-v2.xml
        ${PLASMA_WAYLAND_PROTOCOLS_DIR}/kde-output-management-v2.xml
        ${PLASMA_WAYLAND_PROTOCOLS_DIR}/kde-output-order-v1.xml
    ${private_code_option}
)

target_sources(KSC_KWayland PRIVATE ${wayland_SRCS})

set_target_properties(KSC_KWayland PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kscreen")
set_target_properties(KSC_KWayland PROPERTIES PREFIX "")
target_link_libraries(KSC_KWayland Qt::Core
                                  Qt::DBus
                                  Qt::Gui
                                  Qt::WaylandClient
                                  KF6::Screen
                                  Wayland::Client
)

install(TARGETS KSC_KWayland DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kscreen/)

add_executable(kscreen-doctor main.cpp doctor.cpp dpmsclient.cpp doctor.h dpmsclient.h)

if (QT_MAJOR_VERSION EQUAL "5")
    ecm_add_qtwayland_client_protocol(kscreen-doctor
        PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/dpms.xml
        BASENAME dpms
    )
else()
    qt6_generate_wayland_protocol_client_sources(kscreen-doctor FILES ${PLASMA_WAYLAND_PROTOCOLS_DIR}/dpms.xml)
endif()

target_link_libraries(kscreen-doctor Qt::DBus KF5::Screen KF5::WaylandClient Wayland::Client Qt::WaylandClient)
install(TARGETS kscreen-doctor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

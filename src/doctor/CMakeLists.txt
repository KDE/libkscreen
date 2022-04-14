
ecm_add_qtwayland_client_protocol(wayland_keystate_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/dpms.xml
    BASENAME dpms
)

add_executable(kscreen-doctor main.cpp doctor.cpp dpmsclient.cpp doctor.h dpmsclient.h ${wayland_keystate_SRCS})
target_link_libraries(kscreen-doctor Qt::DBus KF5::Screen KF5::WaylandClient Wayland::Client Qt5::WaylandClient)
install(TARGETS kscreen-doctor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

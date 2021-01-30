
add_executable(kscreen-doctor main.cpp doctor.cpp dpmsclient.cpp)
target_link_libraries(kscreen-doctor Qt::DBus KF5::Screen KF5::WaylandClient)
install(TARGETS kscreen-doctor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

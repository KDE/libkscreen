
add_executable(kscreen-doctor main.cpp doctor.cpp)
target_link_libraries(kscreen-doctor Qt5::DBus KF5::Screen)
install(TARGETS kscreen-doctor ${INSTALL_TARGETS_DEFAULT_ARGS})

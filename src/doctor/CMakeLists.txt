

add_executable(kscreen-doctor main.cpp doctor.cpp)
target_link_libraries(kscreen-doctor KF5::Screen)

install(TARGETS kscreen-doctor ${INSTALL_TARGETS_DEFAULT_ARGS})

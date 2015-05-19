
if (HAVE_DRM AND ENABLE_WAYLAND)
    message(STATUS "Building kscreen-drm test program")
    add_executable(kscreen-drm drmbackend.cpp drmoutput.cpp logind.cpp udev.cpp main.cpp)
    target_link_libraries(kscreen-drm Qt5::Gui Qt5::DBus KF5::Screen Libdrm::Libdrm ${UDEV_LIBS})
endif()



if (HAVE_DRM AND ENABLE_WAYLAND)
    message(STATUS "Building kscreen-drm test program")
    add_executable(kscreen-drm drmbackend.cpp udev.cpp logind.cpp main.cpp)
    target_link_libraries(kscreen-drm Qt5::Gui KF5::Screen Libdrm::Libdrm KF5::CoreAddons)
endif()


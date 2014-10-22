include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/..
                    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(backendlauncher_SRCS
    main.cpp
    backendloader.cpp
    backenddbuswrapper.cpp
)

qt5_add_dbus_adaptor(backendlauncher_SRCS ${CMAKE_SOURCE_DIR}/interfaces/org.kde.KScreen.Backend.xml
                     backenddbuswrapper.h BackendDBusWrapper backendadaptor BackendAdaptor)

add_executable(kscreen_backend_launcher ${backendlauncher_SRCS})

target_link_libraries(kscreen_backend_launcher
    KF5Screen
    Qt5::Core
    Qt5::X11Extras
    Qt5::DBus
)

install(TARGETS kscreen_backend_launcher DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR_KF5})

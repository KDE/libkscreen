set(backendlauncher_SRCS
    main.cpp
    backendloader.cpp
    backenddbuswrapper.cpp
)

ecm_qt_declare_logging_category(backendlauncher_SRCS
                                HEADER kscreen_backendLauncher_debug.h
                                IDENTIFIER KSCREEN_BACKEND_LAUNCHER
                                CATEGORY_NAME org.kde.kscreen.backendLauncher
)

qt_add_dbus_adaptor(backendlauncher_SRCS ${CMAKE_SOURCE_DIR}/interfaces/org.kde.KScreen.Backend.xml
                     backenddbuswrapper.h BackendDBusWrapper backendadaptor BackendAdaptor)
qt_add_dbus_adaptor(backendlauncher_SRCS ${CMAKE_SOURCE_DIR}/interfaces/org.kde.KScreen.xml
                     backendloader.h BackendLoader backendloaderadaptor BackendLoaderAdaptor)

add_executable(kscreen_backend_launcher ${backendlauncher_SRCS})

target_link_libraries(kscreen_backend_launcher
    KF5Screen
    Qt::Core
    Qt::Gui
    Qt::DBus
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(kscreen_backend_launcher Qt::X11Extras)
else()
    target_link_libraries(kscreen_backend_launcher Qt::GuiPrivate)
endif()

install(TARGETS kscreen_backend_launcher
        DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR_KF5}
)

configure_file(org.kde.kscreen.service.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kscreen.service @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kscreen.service
        DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
)

configure_file(plasma-kscreen.service.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/plasma-kscreen.service @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-kscreen.service
        DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
)

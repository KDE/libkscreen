find_package(XCB REQUIRED COMPONENTS XCB RANDR)

include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
                    ${QT_INCLUDES}
)


add_library(KSC_XRandR11 MODULE)
target_sources(KSC_XRandR11 PRIVATE
    xrandr11.cpp
    ../xcbeventlistener.cpp
    ../xcbwrapper.cpp
)

set_target_properties(KSC_XRandR11 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf${QT_MAJOR_VERSION}/kscreen")
set_target_properties(KSC_XRandR11 PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR11 Qt::Core
                                   Qt::Gui
                                   Qt::GuiPrivate # for QX11Info
                                   ${XCB_LIBRARIES}
                                   KF6::Screen
)

install(TARGETS KSC_XRandR11 DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/kscreen/)

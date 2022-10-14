include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
)


add_library(KSC_XRandR MODULE)
target_sources(KSC_XRandR PRIVATE
    xrandr.cpp
    xrandrconfig.cpp
    xrandrcrtc.cpp
    xrandroutput.cpp
    xrandrmode.cpp
    xrandrscreen.cpp
    ../xcbwrapper.cpp
    ../xcbeventlistener.cpp
    ../utils.cpp
)

set_target_properties(KSC_XRandR PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf${QT_MAJOR_VERSION}/kscreen")
set_target_properties(KSC_XRandR PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR Qt::Core
                                 Qt::Gui
                                 ${XCB_LIBRARIES}
                                 KF5::Screen
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(KSC_XRandR Qt::X11Extras)
else()
    target_link_libraries(KSC_XRandR Qt::GuiPrivate)
endif()

install(TARGETS KSC_XRandR DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/kscreen/)

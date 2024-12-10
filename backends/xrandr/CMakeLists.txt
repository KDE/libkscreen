include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
)


add_library(KSC_XRandR MODULE)
target_sources(KSC_XRandR PRIVATE
    xrandr.cpp xrandr.h
    xrandrconfig.cpp xrandrconfig.h
    xrandrcrtc.cpp xrandrcrtc.h
    xrandroutput.cpp xrandroutput.h
    xrandrmode.cpp xrandrmode.h
    xrandrscreen.cpp xrandrscreen.h
    ../xcbwrapper.cpp ../xcbwrapper.h
    ../xcbeventlistener.cpp ../xcbeventlistener.h
    ../utils.cpp ../utils.h
)

set_target_properties(KSC_XRandR PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kscreen")
set_target_properties(KSC_XRandR PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR Qt::Core
                                 Qt::Gui
                                 Qt::GuiPrivate # for QX11Info
                                 ${XCB_LIBRARIES}
                                 KF6::Screen
)

install(TARGETS KSC_XRandR DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kscreen/)

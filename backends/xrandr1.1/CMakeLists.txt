find_package(XCB REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src ${KDE4_INCLUDES} ${CMAKE_BUILD_DIR} ${QT_INCLUDES} ${X11_Xrandr_INCLUDE_PATH} )

set(xrandr11_SRCS
    xrandr11.cpp
    ../xrandr/xrandrx11helper.cpp
)

automoc4_add_library(KSC_XRandR11 MODULE ${xrandr11_SRCS})

set_target_properties(KSC_XRandR11 PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR11 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}  ${QT_QTDBUS_LIBRARY}
                                    ${QJSON_LIBRARIES} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
                                    ${X11_LIBRARIES} ${X11_Xrandr_LIB} ${XCB_XCB_LIBRARIES} ${X11_XCB_LIBRARIES}
                                    ${XCB_RANDR_LIBRARIES} kscreen)

install(TARGETS KSC_XRandR11 DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/kscreen/)
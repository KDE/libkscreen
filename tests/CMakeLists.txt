add_executable(printconfig printconfig.cpp)
target_link_libraries(printconfig Qt5::Gui KF5::Screen)

add_executable(testpnp testplugandplay.cpp testpnp.cpp )
target_link_libraries(testpnp Qt5::Core Qt5::Gui Qt5::X11Extras KF5::Screen)


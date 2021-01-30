add_executable(printconfig testplugandplay.cpp testpnp.cpp)
target_link_libraries(printconfig Qt::Gui KF5::Screen)

add_subdirectory(kwayland)

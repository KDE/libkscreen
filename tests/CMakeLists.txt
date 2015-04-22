add_executable(printconfig testplugandplay.cpp testpnp.cpp)
target_link_libraries(printconfig Qt5::Gui KF5::Screen)

if (ENABLE_WAYLAND)
    add_definitions(-DTEST_DATA="${CMAKE_SOURCE_DIR}/autotests/configs/")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../backends/wayland)

    add_executable(waylandtestserver waylandtestserver.cpp waylandtestservermain.cpp ${CMAKE_SOURCE_DIR}/backends/wayland/waylandconfigreader.cpp)
    target_link_libraries(waylandtestserver Qt5::Gui KF5::Screen KF5::WaylandServer)
endif()


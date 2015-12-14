
add_definitions(-DTEST_DATA="${CMAKE_SOURCE_DIR}/autotests/configs/")

add_executable(waylandtestserver main.cpp waylandtestserver.cpp waylandconfigreader.cpp)
target_link_libraries(waylandtestserver KF5::Screen KF5::WaylandServer)


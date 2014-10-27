add_definitions(-DTEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/configs/")

macro(KSCREEN_ADD_TEST)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} Qt5::Core Qt5::Gui Qt5::Test Qt5::DBus KF5::Screen)
        add_test(kscreen-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach(_testname)
endmacro(KSCREEN_ADD_TEST)

kscreen_add_test(testscreenconfig)
kscreen_add_test(testqscreenbackend)
kscreen_add_test(testconfigserializer)

if (ENABLE_XRANDR_TESTS)
    kscreen_add_test(textxrandr)
endif()

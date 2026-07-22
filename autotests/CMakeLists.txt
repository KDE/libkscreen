add_definitions(-DTEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/configs/")

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/tests/kwayland/)

macro(KSCREEN_ADD_TEST)
    foreach(_testname ${ARGN})
        set(test_SRCS ${_testname}.cpp)
        add_executable(${_testname} ${test_SRCS})
        target_link_libraries(${_testname} Qt::Core Qt::Gui Qt::Test KF6::Screen)
        add_test(NAME kscreen-${_testname}
                 COMMAND dbus-launch $<TARGET_FILE:${_testname}>
        )
        ecm_mark_as_test(${_testname})
    endforeach(_testname)
endmacro(KSCREEN_ADD_TEST)

kscreen_add_test(testscreenconfig)
kscreen_add_test(testconfigserializer)
kscreen_add_test(testconfigmonitor)
kscreen_add_test(testinprocess)
kscreen_add_test(testmodelistchange)
kscreen_add_test(testedid)

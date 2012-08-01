# project(Fake)
#
# cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)
# set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Phonon
# find_package(KScreen REQUIRED)
add_definitions(${QT_DEFINITIONS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES})

set(fake_SRCS
    fake.cpp
    parser.cpp
)

automoc4_add_library(KSC_Fake MODULE ${fake_SRCS})

set_target_properties(KSC_Fake PROPERTIES PREFIX "")
target_link_libraries(KSC_Fake ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QJSON_LIBRARIES} kscreen)

install(TARGETS KSC_Fake DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/kscreen/)



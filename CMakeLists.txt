cmake_minimum_required(VERSION 3.0)

project(libkscreen)
set(PROJECT_VERSION "5.13.80")

set(QT_MIN_VERSION "5.11.0")
set(KF5_MIN_VERSION "5.50.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMSetupVersion)
include(ECMMarkAsTest)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(FeatureSummary)
include(CheckCXXCompilerFlag)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Gui Test X11Extras)

# Wayland backend
find_package(KF5Wayland ${KF5_MIN_VERSION} CONFIG REQUIRED)
add_feature_info("KF5Wayland" KF5Wayland_FOUND "Required for building libkscreen's KWayland backend")

# xrandr backend

find_package(XCB COMPONENTS XCB RANDR)
set_package_properties(XCB PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Required for building XRandR backends"
)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

# library setup

add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
)

set(KF5_VERSION ${PROJECT_VERSION}) #When we are happy with the api, we can sync with frameworks
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KSCREEN
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kscreen_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ScreenConfigVersion.cmake"
                        SOVERSION 7)

check_cxx_compiler_flag(-fvisibility=hidden _HAVE_VISIBILITY)
if (_HAVE_VISIBILITY AND NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

  check_cxx_compiler_flag(-fvisibility-inlines-hidden _HAVE_VISIBILITY_INLINES)
  if (_HAVE_VISIBILITY_INLINES)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
  endif (_HAVE_VISIBILITY_INLINES)
endif (_HAVE_VISIBILITY AND NOT WIN32)

add_subdirectory(src)
add_subdirectory(backends)
if(BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Screen")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Screen_QCH
        FILE KF5ScreenQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5ScreenQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5ScreenConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5ScreenConfig.cmake"
                              INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ScreenConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ScreenConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT
  KF5ScreenTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE KF5ScreenTargets.cmake
  NAMESPACE KF5::
  COMPONENT Devel
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/kscreen_version.h"
  DESTINATION "${KF5_INCLUDE_INSTALL_DIR}"
  COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

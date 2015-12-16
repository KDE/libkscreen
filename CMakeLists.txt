cmake_minimum_required(VERSION 2.8.12)

project(libkscreen)
set(PROJECT_VERSION "5.5.90")

find_package(ECM 5.14.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(ECMSetupVersion)
include(ECMPackageConfigHelpers)
include(ECMMarkAsTest)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)
include(CheckCXXCompilerFlag)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core DBus Gui Test X11Extras)

# Wayland backend
find_package(KF5Wayland CONFIG)
add_feature_info("KF5Wayland" KF5Wayland_FOUND "Required for building libkscreen's KWayland backend")

# xrandr backend

find_package(XCB COMPONENTS XCB RANDR)
set_package_properties(XCB PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Required for building XRandR backends"
)

# library setup

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
add_subdirectory(autotests)
add_subdirectory(tests)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Screen")
ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5ScreenConfig.cmake.in"
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

############################################ apidox ############################################

option(LIBKSCREEN_BUILD_API_DOCS "Build libkscreen API documentation")

if(LIBKSCREEN_BUILD_API_DOCS)
  find_package(Doxygen)

  if(DOXYGEN_EXECUTABLE)
    configure_file(${libkscreen_SOURCE_DIR}/.Doxyfile.cmake ${libkscreen_BINARY_DIR}/Doxyfile)

    if(EXISTS ${QT_DOC_DIR}/html)
      set(QTDOCS "${QT_DOC_DIR}/html")
    else(EXISTS ${QT_DOC_DIR}/html)
      set(QTDOCS "http://doc.qt.nokia.com/latest/")
    endif(EXISTS ${QT_DOC_DIR}/html)

    add_custom_target(
      apidox ALL
      COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
      COMMAND doc/html/installdox -l qt4.tag@${QTDOCS} doc/html/*.html)
  endif(DOXYGEN_EXECUTABLE)
endif(LIBKSCREEN_BUILD_API_DOCS)

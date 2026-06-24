cmake_minimum_required(VERSION 3.16)

project(bluecurve-kdecoration LANGUAGES CXX)

set(KF6_MIN_VERSION "6.0.0")
set(QT_MIN_VERSION "6.0.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(CMakePackageConfigHelpers)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(KDEInstallDirs6)
include(KDECMakeSettings)

# Qt/KF includes
find_package(Qt6 REQUIRED COMPONENTS DBus Widgets Core)
find_package(KF6 REQUIRED COMPONENTS CoreAddons)
find_package(KDecoration3 REQUIRED)

# Set up GTK 3 (needed for installation directories)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 gtk+-3.0)
if (NOT GTK3_FOUND)
  message(FATAL_ERROR "GTK 3 development libraries were not found.")
endif()
pkg_get_variable(GTK3_PREFIX gtk+-3.0 prefix)

add_library(bluecurve-decoration MODULE
  kwin/bluecurvedecoration.cpp
)

set_target_properties(bluecurve-decoration PROPERTIES
  OUTPUT_NAME com.github.neeeeow.bluecurve
)

target_link_libraries(bluecurve-decoration
  PRIVATE
  Qt6::DBus
  Qt6::Widgets
  KF6::CoreAddons
  KDecoration3::KDecoration
)

install(TARGETS bluecurve-decoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KDECORATION_PLUGIN_DIR})
install(DIRECTORY plasma/Bluecurve DESTINATION ${KDE_INSTALL_DATADIR}/plasma/desktoptheme)
install(DIRECTORY gtk/Bluecurve-KDE DESTINATION ${GTK3_PREFIX}/share/themes)

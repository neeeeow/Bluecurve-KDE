cmake_minimum_required(VERSION 3.16)

project(bluecurve-kdecoration LANGUAGES CXX)

set(KF5_MIN_VERSION "5.102.0")
set(KF6_MIN_VERSION "6.18.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(QT5_MIN_VERSION "5.15.2")
set(QT_MIN_VERSION "6.9.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ECM setup
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
find_package(KF6 REQUIRED COMPONENTS
  CoreAddons)
find_package(KDecoration3 REQUIRED)

add_library(bluecurve-decoration MODULE
  src/bluecurvedecoration.cpp
)

set_target_properties(bluecurve-decoration PROPERTIES
  PREFIX ""
  OUTPUT_NAME org.kde.bluecurve
)

target_link_libraries(bluecurve-decoration
  PRIVATE
  Qt6::DBus
  Qt6::Widgets
  KF6::CoreAddons
  KDecoration3::KDecoration
)

install(TARGETS bluecurve-decoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KDECORATION_PLUGIN_DIR})

# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

project(kjournald LANGUAGES CXX)
set(KJOURNALD_VERSION 0.0.3)

cmake_minimum_required(VERSION 3.16)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FeatureSummary)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050e00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055600
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

find_package(ECM 5.44.0 NO_MODULE)
set_package_properties(ECM PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Extra CMake Modules."
    URL "https://invent.kde.org/frameworks/extra-cmake-modules")
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMAddTests)
include(ECMEnableSanitizers)
include(ECMCoverageOption)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECMakeSettings)

ecm_setup_version(${KJOURNALD_VERSION}
    VARIABLE_PREFIX KJOURNALD
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kjournald_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KJournaldConfigVersion.cmake"
    SOVERSION 0
)

enable_testing()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

set(REQUIRED_QT_VERSION 5.15.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} COMPONENTS Core Quick Test REQUIRED)

add_subdirectory(lib)
add_subdirectory(browser)
add_subdirectory(autotests)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KJournald")
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KJournaldConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KJournaldConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KJournaldConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KJournaldConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kjournald_version.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)

install(EXPORT KJournaldTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KJournaldTargets.cmake)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

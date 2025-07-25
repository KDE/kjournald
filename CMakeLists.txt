# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

cmake_minimum_required(VERSION 3.19)

set(RELEASE_SERVICE_VERSION_MAJOR "25")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(KJOURNALD_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
set(KJOURNALD_SOVERSION 0)

project(kjournald VERSION ${KJOURNALD_VERSION})

# generate QML LS integration
set(QT_QML_GENERATE_QMLLS_INI ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FeatureSummary)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x060800
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060800
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x0060800
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060800
)

set(KF_VERSION 6.8.0)
find_package(ECM ${KF_VERSION} NO_MODULE)
set_package_properties(ECM PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Extra CMake Modules."
    URL "https://invent.kde.org/frameworks/extra-cmake-modules")
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMAddTests)
include(ECMEnableSanitizers)
include(ECMCoverageOption)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(KDEInstallDirs6)
include(KDECMakeSettings)
include(KDEClangFormat)
include(ECMQmlModule)

ecm_setup_version(${KJOURNALD_VERSION}
    VARIABLE_PREFIX KJOURNALD
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kjournald_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KJournaldConfigVersion.cmake"
    SOVERSION ${KJOURNALD_SOVERSION}
)

find_package(KF6 ${KF_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Crash
    Config
    I18n
    Kirigami
)
add_definitions(-DTRANSLATION_DOMAIN=\"kjournald\")

find_package(KF6KirigamiAddons 1.4.0 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET libsystemd)

find_package(Qt6 6.5.0 REQUIRED COMPONENTS
    Core
    Quick
    QuickControls2
    Test
    Widgets
)

option(INSTALL_EXPERIMENTAL_HEADERS "Install headers for libkjournald (no stable API yet)" OFF)

enable_testing()

add_subdirectory(org/kde/kjournald/)
add_subdirectory(org/kde/kjournaldbrowser/)
add_subdirectory(browser)
add_subdirectory(autotests)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KJournald")
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KJournaldConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KJournaldConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

if(INSTALL_EXPERIMENTAL_HEADERS)
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
endif()

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

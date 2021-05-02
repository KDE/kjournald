# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

project(kjournald LANGUAGES CXX)
set(KJOURNALD_VERSION 0.0.1)

cmake_minimum_required(VERSION 3.10)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FeatureSummary)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

find_package(ECM 5.44.0 NO_MODULE)
set_package_properties(ECM PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Extra CMake Modules."
    URL "https://invent.kde.org/frameworks/extra-cmake-modules")
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMAddTests)
include(ECMEnableSanitizers)
include(ECMCoverageOption)
include(KDEInstallDirs)
include(KDECMakeSettings)

enable_testing()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

set(REQUIRED_QT_VERSION 5.12.0)
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
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/KJournaldConfigVersion.cmake"
    VERSION ${KJOURNALD_VERSION}
    COMPATIBILITY SameMinorVersion # API still changes a lot, better caution about compatibility; TODO set to MAJOR
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KJournaldConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KJournaldConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}")

install(EXPORT KJournaldTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KJournaldTargets.cmake)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

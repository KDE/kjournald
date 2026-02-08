/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2024 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef FILTER_H
#define FILTER_H

#include "kjournald_export.h"
#include <QQmlEngine>
#include <QString>
#include <optional>

/**
 * Configuration struct for the filter configuration if a view model.
 */
class KJOURNALD_EXPORT Filter
{
    Q_GADGET

    /**
     * filter for message priorities
     */
    Q_PROPERTY(int priority READ priorityFilterInt WRITE setPriorityFilter RESET resetPriorityFilter)
    /**
     * filter list for systemd units
     **/
    Q_PROPERTY(QStringList boots READ bootFilter WRITE setBootFilter)
    /**
     * filter list for systemd user units
     **/
    Q_PROPERTY(QStringList userUnits READ systemdUserUnitFilter WRITE setSystemdUserUnitFilter)
    /**
     * filter list for systemd system units
     **/
    Q_PROPERTY(QStringList systemUnits READ systemdSystemUnitFilter WRITE setSystemdSystemUnitFilter)
    /**
     * filter list for executables (see journald '_EXE' field)
     **/
    Q_PROPERTY(QStringList exes READ exeFilter WRITE setExeFilter)
    /**
     * if set to true, Kernel messages are added to the log output
     **/
    Q_PROPERTY(bool kernel READ areKernelMessagesEnabled WRITE setKernelMessagesEnabled)

    QML_ANONYMOUS

public:
    /**
     * \return the currently selected priority threshold for displayed log entries
     */
    [[nodiscard]] std::optional<quint8> priorityFilter() const;

    /**
     * \return the currently selected priority threshold for displayed log entries
     */
    [[nodiscard]] int priorityFilterInt() const;

    /**
     * \brief Filter messages such that only messages with this and higher priority are provided
     *
     * \note Non-systemd services may not follow systemd's priority values
     *
     * \param priority the minimal priority for messages that shall be provided by model
     */
    void setPriorityFilter(int priority);

    /**
     * reset priority filter
     */
    void resetPriorityFilter();

    /**
     * \return list of currently set boot ids for filtering
     */
    [[nodiscard]] QStringList bootFilter() const;

    /**
     * \brief Configure for which boots messages shall be shown
     *
     * If no boot id is configured, this filter is deactivated. The given values are compared
     * to the _BOOT_ID journal value.
     *
     * \param bootFilter list of boot ids
     */
    void setBootFilter(const QStringList &bootFilter);

    /**
     * \return the list of enabled user units
     */
    [[nodiscard]] QStringList systemdUserUnitFilter() const;

    /**
     * \brief Configure for which systemd user units messages shall be shown
     *
     * If no unit is configured, this filter is deactivated. The given values are compared
     * to the _SYSTEMD_USER_UNIT journal value.
     *
     * \param units list of user units
     */
    void setSystemdUserUnitFilter(const QStringList &units);

    /**
     * \return the list of enabled system units
     */
    [[nodiscard]] QStringList systemdSystemUnitFilter() const;

    /**
     * \brief Configure for which systemd system units messages shall be shown
     *
     * If no unit is configured, this filter is deactivated. The given values are compared
     * to the _SYSTEMD_UNIT journal value.
     *
     * \param units list of system units
     */
    void setSystemdSystemUnitFilter(const QStringList &units);

    /**
     * \return the list of enabled processes
     */
    [[nodiscard]] QStringList exeFilter() const;

    /**
     * Configure for which executable messages shall be shown
     *
     * If no executable is configured, this filter is deactivated. The given values are compared
     * to the _EXE journal value.
     *
     * \param exeFilter list of executable paths
     */
    void setExeFilter(const QStringList &exeFilter);

    /**
     * \return true if kernel log entries shall be included, otherwise false
     */
    [[nodiscard]] bool areKernelMessagesEnabled() const;

    /**
     * \brief Configure if Kernel messages shall be included
     *
     * Per default, Kernel messages are deactivated.
     *
     * \param showKernelMessages parameter that defines if Kernel messages shall be shown
     */
    void setKernelMessagesEnabled(bool showKernelMessages);

private:
    std::optional<quint8> mPriority{std::nullopt};
    QStringList mBootFilter;
    QStringList mExeFilter;
    QStringList mUserUnitFilter;
    QStringList mSystemUnitFilter;
    bool mEnableKernelMessages{false};
};

QDebug operator<<(QDebug dbg, const Filter &c);

#endif

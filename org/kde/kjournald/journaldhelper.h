/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDHELPER_H
#define JOURNALDHELPER_H

#include "kjournald_export.h"
#include <QDateTime>
#include <QDebugStateSaver>
#include <QObject>
#include <QVector>
#include <ijournal.h>

class KJOURNALD_EXPORT JournaldHelper
{
    Q_GADGET
public:
    /**
     * @brief Basic information of a boot
     */
    struct BootInfo {
        QString mBootId; //!< unique identifier of the boot
        QDateTime mSince; //!< time of oldest log entry for the specific boot
        QDateTime mUntil; //!< time of newest log entry for the specific boot
    };

    /**
     * @brief Enumeration of most prominent field contents
     *
     * Names of these fields are identitical to their actual names with the only difference
     * that the "_" is removed from the reserved fields for Q_ENUM compatability.
     */
    enum class Field {
        // user fields
        MESSAGE,
        MESSAGE_ID,
        PRIORITY,
        CODE_FILE,
        CODE_LINE,
        CODE_FUNC,
        // trusted fields
        _BOOT_ID,
        _EXE,
        _SYSTEMD_CGROUP,
        _SYSTEMD_SLICE,
        _SYSTEMD_UNIT,
        _SYSTEMD_USER_UNIT,
        _SYSTEMD_USER_SLICE,
        _SYSTEMD_SESSION,
        _SYSTEMD_OWNER_UID,
        _TRANSPORT,
    };
    Q_ENUM(Field)

    /**
     * @brief Query unique field values fro given field
     *
     * This method returns all unique values provided by a defined journald database
     * field, e.g. the list of all boot-ids or the list of all services.
     *
     * This method wraps sd_journal_query_unique, which according to its documentation
     * ignores any add_match settings of the used @a journal. Yet this may change in the
     * future and it is encouraged to use a separate journal object to request unique valus.
     *
     * @param journal the wrapper object for an sd_journal instance
     * @param field the requested field
     * @return the list of unique field contents
     */
    static QVector<QString> queryUnique(const IJournal &journal, Field field);

    /**
     * @copydoc JournaldHelper::queryUnique
     */
    static QVector<QString> queryUnique(std::shared_ptr<IJournal> journal, Field field);

    /**
     * @brief Query boot information for @p journal
     *
     * @return ordered list of boots (first is earliest boot in time)
     */
    static QVector<BootInfo> queryOrderedBootIds(const IJournal &journal);

    /**
     * @brief Mapper method that maps from field enum to textual representation
     *
     * @param field the field enum for which the textual repesentation is requested
     * @return string representation of enum
     */
    static QString mapField(Field field);

    /**
     * @brief Cleanup typical decorations from strings as found in journald databases
     * @param string the string to process
     * @return cleaned string
     */
    static QString cleanupString(const QString &string);
};

QDebug operator<<(QDebug debug, const JournaldHelper::BootInfo &bootInfo);

#endif // JOURNALDHELPER_H

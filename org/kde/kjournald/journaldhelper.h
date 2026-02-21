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
#include <ijournalprovider.h>

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
     * @note that calling this method leads to add_match calls to the provided journal and will
     * change further filter results.
     *
     * @param journal valid pointer to sd_journal instance
     * @param field the requested field
     * @return the list of unique field contents
     */
    static QList<QString> queryUnique(sd_journal *journal, Field field);

    /**
     * @brief Collect unique field entries for specified boot
     *
     * This method collects all distinct field values for the given boot. Note that the computational
     * complexity is O(N) with N number of entries in the journal. Also beware, that read pointer and
     * filter options change of the used sd_journal object.
     *
     * @param journal the openend journal
     * @param bootId the boot ID
     * @param field the field identifier
     * @return list of distinct values for field
     */
    static QList<QString> queryUnique(sd_journal *journal, QAnyStringView bootId, Field field);

    /**
     * @brief Collect multiple unique field entries for specified boot
     *
     * This method collects all distinct field values for the given boot of multiple fields simultaneously
     * Note that the computational complexity is O(N) with N number of entries in the journal. Also beware,
     * that read pointer and filter options change of the used sd_journal object.
     *
     * Prefer this method over requesting every field seperately.
     *
     * @param journal the openend journal
     * @param bootId the boot ID
     * @param field the field identifier
     * @return list of distinct values for field
     */
    static QMap<Field, QStringList> queryUnique(sd_journal *journal, QAnyStringView bootId, QList<Field> fields);

    /**
     * @brief Query boot information for @p journal
     *
     * @return ordered list of boots (first is earliest boot in time)
     */
    static QList<BootInfo> queryOrderedBootIds(sd_journal *journal);

    /**
     * @brief Mapper method that maps from field enum to textual representation
     *
     * @param field the field enum for which the textual repesentation is requested
     * @return string representation of enum
     */
    static QLatin1StringView mapField(JournaldHelper::Field field);

    /**
     * @brief Cleanup typical decorations from strings as found in journald databases
     * @param string the string to process
     * @return cleaned string
     */
    static QString cleanupString(const QString &string);

    static constexpr QLatin1String ID_MESSAGE{"MESSAGE"};
    static constexpr QLatin1String ID_MESSAGE_ID{"MESSAGE_ID"};
    static constexpr QLatin1String ID_PRIORITY{"PRIORITY"};
    static constexpr QLatin1String ID_CODE_FILE{"CODE_FILE"};
    static constexpr QLatin1String ID_CODE_LINE{"CODE_LINE"};
    static constexpr QLatin1String ID_CODE_FUNC{"CODE_FUNC"};
    static constexpr QLatin1String ID__BOOT_ID{"_BOOT_ID"};
    static constexpr QLatin1String ID__EXE{"_EXE"};
    static constexpr QLatin1String ID__SYSTEMD_CGROUP{"_SYSTEMD_CGROUP"};
    static constexpr QLatin1String ID__SYSTEMD_SLICE{"_SYSTEMD_SLICE"};
    static constexpr QLatin1String ID__SYSTEMD_UNIT{"_SYSTEMD_UNIT"};
    static constexpr QLatin1String ID__SYSTEMD_USER_UNIT{"_SYSTEMD_USER_UNIT"};
    static constexpr QLatin1String ID__SYSTEMD_USER_SLICE{"_SYSTEMD_USER_SLICE"};
    static constexpr QLatin1String ID__SYSTEMD_SESSION{"_SYSTEMD_SESSION"};
    static constexpr QLatin1String ID__SYSTEMD_OWNER_UID{"_SYSTEMD_OWNER_UID"};
    static constexpr QLatin1String ID__TRANSPORT{"_TRANSPORT"};
};

QDebug operator<<(QDebug debug, const JournaldHelper::BootInfo &bootInfo);

#endif // JOURNALDHELPER_H

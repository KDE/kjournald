/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journaldhelper.h"
#include "kjournaldlib_log_general.h"
#include <QDebug>
#include <QMetaEnum>
#include <QString>
#include <systemd/sd-journal.h>

QList<QString> JournaldHelper::queryUnique(sd_journal *journal, Field field)
{
    if (!journal) {
        qCritical() << "Failed query unique, sd_journal is null";
        return {};
    }

    QList<QString> dataList;
    const void *data;
    size_t length{0};
    int result{0};

    const QString fieldString = mapField(field);

    result = sd_journal_query_unique(journal, qUtf8Printable(fieldString));
    if (result < 0) {
        qCritical() << "Failed to query journal:" << strerror(-result);
        return dataList;
    }
    const int fieldLength = fieldString.length() + 1;
    SD_JOURNAL_FOREACH_UNIQUE(journal, data, length)
    {
        QString dataStr = QString::fromUtf8(static_cast<const char *>(data), length);
        dataList << dataStr.remove(0, fieldLength);
    }
    return dataList;
}

QList<JournaldHelper::BootInfo> JournaldHelper::queryOrderedBootIds(sd_journal *journal)
{
    if (!journal) {
        qCritical() << "Failed query ordered boot ids, sd_journal is null";
        return {};
    }
    QList<JournaldHelper::BootInfo> boots;
    const QList<QString> bootIds = JournaldHelper::queryUnique(journal, Field::_BOOT_ID);
    for (const QString &id : bootIds) {
        int result{0};
        uint64_t time{0};

        sd_journal_flush_matches(journal);
        QString filterExpression = QLatin1String("_BOOT_ID=") + id;
        result = sd_journal_add_match(journal, filterExpression.toStdString().c_str(), filterExpression.length());
        if (result < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Failed add filter:" << strerror(-result);
            continue;
        }

        QDateTime since;
        result = sd_journal_seek_head(journal);
        if (result < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Failed to seek head:" << strerror(-result);
            continue;
        }
        result = sd_journal_next(journal);
        if (result < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Failed to obtain first entry:" << strerror(-result);
            continue;
        }
        result = sd_journal_get_realtime_usec(journal, &time);
        if (result == 0) {
            since.setMSecsSinceEpoch(time / 1000);
        } else {
            qCCritical(KJOURNALDLIB_GENERAL) << "Failed to obtain time:" << strerror(-result);
        }

        QDateTime until;
        result = sd_journal_seek_tail(journal);
        if (result < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Failed to seek tail:" << strerror(-result);
            continue;
        }
        result = sd_journal_previous(journal);
        if (result < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Failed to obtain first entry:" << strerror(-result);
        }
        result = sd_journal_get_realtime_usec(journal, &time);
        if (result == 0) {
            until.setMSecsSinceEpoch(time / 1000);
        } else {
            qCCritical(KJOURNALDLIB_GENERAL) << "Failed to obtain time:" << strerror(-result);
        }

        if (since.isValid() && until.isValid()) {
            boots << BootInfo{id, since, until};
        } else {
            qCCritical(KJOURNALDLIB_GENERAL) << "Could not correctly parse start/end time for boot" << id << "skipping from list";
        }
    }

    std::sort(boots.begin(), boots.end(), [](const JournaldHelper::BootInfo &lhs, const JournaldHelper::BootInfo &rhs) {
        return lhs.mSince < rhs.mSince;
    });

    return boots;
}

QList<QString> JournaldHelper::queryUnique(sd_journal *journal, QAnyStringView bootId, Field field)
{
    return queryUnique(journal, bootId, QList({field})).value(field);
}

QMap<JournaldHelper::Field, QStringList> JournaldHelper::queryUnique(sd_journal *journal, QAnyStringView bootId, QList<Field> fields)
{
    auto generateIds = [](const auto &fields) {
        QVarLengthArray<QLatin1StringView, 1> ids(fields.size());
        for (int i = 0; i < fields.size(); ++i) {
            ids[i] = mapField(fields[i]);
        }
        return ids;
    };
    const auto fieldIds = generateIds(fields);
    QVarLengthArray<QSet<QString>, 1> entrySets(fields.size());

    const char *data{nullptr};
    size_t length{0};
    int result{0};

    sd_journal_flush_matches(journal);
    sd_journal_seek_head(journal);

    QString filterExpression = QString(QStringLiteral("%1=%2")).arg(mapField(Field::_BOOT_ID), bootId);
    result = sd_journal_add_match(journal, filterExpression.toLocal8Bit().constData(), filterExpression.length());
    if (result < 0) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Failed add filter:" << strerror(-result);
        return {};
    }
    while (sd_journal_next(journal) > 0) {
        for (int i = 0; i < fields.size(); ++i) {
            result = sd_journal_get_data(journal, fieldIds[i].data(), (const void **)&data, &length);
            if (result == 0) {
                // entry starts with "_ID=", remove that part
                entrySets[i].insert(QString::fromUtf8(data, length).mid(fieldIds[i].size() + 1));
            }
        }
    }

    QMap<Field, QStringList> entryMap;
    for (int i = 0; i < fields.size(); ++i) {
        entryMap.insert(fields.at(i), entrySets.at(i).values());
    }
    return entryMap;
}

constexpr QLatin1StringView JournaldHelper::mapField(Field field)
{
    switch (field) {
    case Field::MESSAGE:
        return ID_MESSAGE;
    case Field::MESSAGE_ID:
        return ID_MESSAGE_ID;
    case Field::PRIORITY:
        return ID_PRIORITY;
    case Field::CODE_FILE:
        return ID_CODE_FILE;
    case Field::CODE_LINE:
        return ID_CODE_LINE;
    case Field::CODE_FUNC:
        return ID_CODE_FUNC;
    case Field::_BOOT_ID:
        return ID__BOOT_ID;
    case Field::_EXE:
        return ID__EXE;
    case Field::_SYSTEMD_CGROUP:
        return ID__SYSTEMD_CGROUP;
    case Field::_SYSTEMD_SLICE:
        return ID__SYSTEMD_SLICE;
    case Field::_SYSTEMD_UNIT:
        return ID__SYSTEMD_UNIT;
    case Field::_SYSTEMD_USER_UNIT:
        return ID__SYSTEMD_USER_UNIT;
    case Field::_SYSTEMD_USER_SLICE:
        return ID__SYSTEMD_USER_SLICE;
    case Field::_SYSTEMD_SESSION:
        return ID__SYSTEMD_SESSION;
    case Field::_SYSTEMD_OWNER_UID:
        return ID__SYSTEMD_OWNER_UID;
    case Field::_TRANSPORT:
        return ID__TRANSPORT;
    }
}

QString JournaldHelper::cleanupString(const QString &string)
{
    QString cleaned;
    cleaned.reserve(string.size());
    int i = 0;
    while (i < string.size()) {
        if (string.at(i) == QLatin1Char('\u0001')) { // skip certain unicode characters
            ++i;
            continue;
        } else if (i + 3 >= string.size() || string.at(i) != QLatin1Char('\\') || string.at(i + 1) != QLatin1Char('x')) {
            cleaned.append(string.at(i));
            ++i;
            continue;
        } else {
            // handle ascii escape sequence, eg. "\x2d" for "-"
            bool ok;
            auto character = QChar::fromLatin1(QStringView(string).mid(i + 2, 2).toUInt(&ok, 16));
            Q_ASSERT(ok);
            cleaned.append(character);
            i += 4;
            continue;
        }
    }
    return cleaned;
}

QDebug operator<<(QDebug debug, const JournaldHelper::BootInfo &bootInfo)
{
    QDebugStateSaver saver(debug);
    debug.noquote() << bootInfo.mBootId << ':' << bootInfo.mSince.toString(Qt::DateFormat::ISODateWithMs) << '-'
                    << bootInfo.mUntil.toString(Qt::DateFormat::ISODateWithMs);
    return debug;
}

#include "moc_journaldhelper.cpp"

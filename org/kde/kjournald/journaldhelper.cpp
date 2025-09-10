/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journaldhelper.h"
#include "kjournaldlib_log_general.h"
#include <QDebug>
#include <QMetaEnum>
#include <systemd/sd-journal.h>

QVector<QString> JournaldHelper::queryUnique(sd_journal *journal, Field field)
{
    if (!journal) {
        qCritical() << "Failed query unique, sd_journal is null";
        return {};
    }

    QVector<QString> dataList;
    const void *data;
    size_t length;
    int result;

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

QVector<JournaldHelper::BootInfo> JournaldHelper::queryOrderedBootIds(sd_journal *journal)
{
    if (!journal) {
        qCritical() << "Failed query ordered boot ids, sd_journal is null";
        return {};
    }
    QVector<JournaldHelper::BootInfo> boots;
    QVector<QString> bootIds = JournaldHelper::queryUnique(journal, Field::_BOOT_ID);
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

QString JournaldHelper::mapField(Field field)
{
    static QMetaEnum metaEnum = QMetaEnum::fromType<JournaldHelper::Field>();
    return QString::fromLatin1(metaEnum.valueToKey(static_cast<int>(field)));
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

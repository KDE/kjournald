/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "logentry.h"

LogEntry::LogEntry(const QDateTime &date,
                   quint64 monotonicTimestamp,
                   const QString &id,
                   const QString &message,
                   const QString &unit,
                   const QString &bootId,
                   const QString &exe,
                   int priority,
                   const QString &cursor)
    : m_id{id}
    , m_date{date}
    , m_monotonicTimestamp{monotonicTimestamp}
    , m_unit{unit}
    , m_bootId{bootId}
    , m_exe{exe}
    , m_priority{priority}
    , m_cursor{cursor}
{
    setMessage(message);
}

bool LogEntry::matches(const QString &needle, bool caseSensitive) const
{
    if (needle.isEmpty()) {
        return false;
    }
    const Qt::CaseSensitivity caseSensitiveEnum = caseSensitive ? Qt::CaseSensitivity::CaseSensitive : Qt::CaseSensitivity::CaseInsensitive;
    if (m_message.contains(needle, caseSensitiveEnum)) {
        return true;
    } else {
        return false;
    }
}

void LogEntry::setMessage(const QString &message)
{
    // TODO handle cleanup of arbitrary color codes
    // cleanup color codes
    m_message = message;
    m_message.remove(QLatin1String("\u001B[96m")).remove(QLatin1String("\u001B[0m")).remove(QLatin1String("\u001B[93m")).remove(QLatin1String("\u001B[31m"));
}

void LogEntry::setDate(const QDateTime &date)
{
    m_date = date;
}

void LogEntry::setMonotonicTimestamp(quint64 timestamp)
{
    m_monotonicTimestamp = timestamp;
}

void LogEntry::setPriority(int priority)
{
    m_priority = priority;
}

void LogEntry::setId(const QString &id)
{
    m_id = id;
}

void LogEntry::setBootId(const QString &bootId)
{
    m_bootId = bootId;
}

void LogEntry::setUnit(const QString &unit)
{
    m_unit = unit;
}

void LogEntry::setExe(const QString &exe)
{
    m_exe = exe;
}

void LogEntry::setCursor(const QString &cursor)
{
    m_cursor = cursor;
}

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <QObject>
#include <QQmlEngine>

class LogEntry
{
    Q_GADGET

    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(QString message READ message WRITE setMessage)
    Q_PROPERTY(QDateTime date READ date WRITE setDate)
    Q_PROPERTY(quint64 monotonicTimestamp READ monotonicTimestamp)
    Q_PROPERTY(int priority READ priority WRITE setPriority)
    Q_PROPERTY(QString bootId READ bootId WRITE setBootId)
    Q_PROPERTY(QString unit READ unit WRITE setUnit)
    Q_PROPERTY(QString exe READ exe WRITE setExe)
    Q_PROPERTY(QString cursor READ cursor WRITE setCursor)

    QML_VALUE_TYPE(entry)

public:
    LogEntry() = default;
    ~LogEntry() = default;
    /** convenience constructor **/
    LogEntry(const QDateTime &date,
             quint64 monotonicTimestamp,
             const QString &id,
             const QString &message,
             const QString &unit,
             const QString &bootId,
             const QString &exe,
             int priority,
             const QString &cursor);

    Q_INVOKABLE bool matches(const QString &needle, bool caseSensitive) const;

    inline QString message() const
    {
        return m_message;
    }
    void setMessage(const QString &message);
    inline QDateTime date() const
    {
        return m_date;
    }
    void setDate(const QDateTime &date);
    inline quint64 monotonicTimestamp() const
    {
        return m_monotonicTimestamp;
    }
    void setMonotonicTimestamp(quint64 timestamp);
    inline int priority() const
    {
        return m_priority;
    }
    void setPriority(int priority);
    inline QString id() const
    {
        return m_id;
    }
    void setId(const QString &id);
    inline QString bootId() const
    {
        return m_bootId;
    }
    void setBootId(const QString &bootId);
    inline QString unit() const
    {
        return m_unit;
    }
    void setUnit(const QString &unit);
    inline QString exe() const
    {
        return m_exe;
    }
    void setExe(const QString &exe);
    inline QString cursor() const
    {
        return m_cursor;
    }
    void setCursor(const QString &cursor);

private:
    QString m_id;
    QString m_message;
    QDateTime m_date;
    quint64 m_monotonicTimestamp{0};
    int m_priority{0};
    QString m_bootId;
    QString m_unit;
    QString m_exe;
    QString m_cursor;
};

#endif // LOGENTRY_H

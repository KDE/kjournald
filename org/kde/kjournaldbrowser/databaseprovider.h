/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021-2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef DATABASEPROVIDER_H
#define DATABASEPROVIDER_H

#include "systemdjournalremote.h"
#include <QObject>
#include <QQmlEngine>
#include <QSettings>

/**
 * @brief Provides path to current journald database
 */
class DatabaseProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DatabaseProvider::Mode mode READ mode NOTIFY journalPathChanged)

    Q_PROPERTY(QString journalPath READ journalPath NOTIFY journalPathChanged)
    /**
     * @note this path can either be a directory or a file
     */
    Q_PROPERTY(QString localJournalPath READ localJournalPath NOTIFY journalPathChanged)
    Q_PROPERTY(QString remoteJournalUrl READ remoteJournalUrl NOTIFY journalPathChanged)
    Q_PROPERTY(quint32 remoteJournalPort READ remoteJournalPort NOTIFY journalPathChanged)

    QML_ELEMENT
    QML_SINGLETON

public:
    enum class Mode {
        LOCALFOLDER, //!< local available journald folder
        SYSTEM, //!< local system journald database
        REMOTE, //!< reading from remote port
    };
    Q_ENUM(Mode);

    explicit DatabaseProvider(QObject *parent = nullptr);
    ~DatabaseProvider() override;

    DatabaseProvider::Mode mode() const;

    Q_INVOKABLE void setSystemJournal();
    Q_INVOKABLE void setLocalJournalPath(const QString &path);
    Q_INVOKABLE void setRemoteJournalUrl(const QString &url, quint32 port);

    QString journalPath() const;
    QString localJournalPath() const;
    QString remoteJournalUrl() const;
    quint32 remoteJournalPort() const;

Q_SIGNALS:
    void journalPathChanged();

private:
    void initRemoteJournal();

    Mode mMode{Mode::SYSTEM};
    QString mJournalPath;
    QString mRemoteJournalUrl;
    quint32 mRemoteJournalPort{0};
    std::unique_ptr<SystemdJournalRemote> mRemoteJournal;
};

#endif

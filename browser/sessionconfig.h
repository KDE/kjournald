/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef SESSIONCONFIG_H
#define SESSIONCONFIG_H

#include "systemdjournalremote.h"
#include <QObject>
#include <QQmlEngine>
#include <QSettings>

/**
 * @brief Central config for current session
 */
class SessionConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SessionConfig::Mode sessionMode READ mode WRITE setMode NOTIFY modeChanged)
    /**
     * @note this path can either be a directory or a file
     */
    Q_PROPERTY(QString localJournalPath READ localJournalPath WRITE setLocalJournalPath NOTIFY localJournalPathChanged)
    Q_PROPERTY(QString remoteJournalUrl READ remoteJournalUrl WRITE setRemoteJournalUrl NOTIFY remoteJournalUrlChanged)
    Q_PROPERTY(quint32 remoteJournalPort READ remoteJournalPort WRITE setRemoteJournalPort NOTIFY remoteJournalPortChanged)

public:
    enum class Mode {
        LOCALFOLDER, //!< local available journald folder
        SYSTEM, //!< local system journald database
        REMOTE, //!< reading from remote port
    };
    Q_ENUM(Mode);

    explicit SessionConfig(QObject *parent = nullptr);
    ~SessionConfig() override;

    void setMode(Mode mode);

    SessionConfig::Mode mode() const;

    void setLocalJournalPath(const QString &path);

    QString localJournalPath() const;

    void setRemoteJournalUrl(const QString &url);

    QString remoteJournalUrl() const;

    void setRemoteJournalPort(quint32 port);

    quint32 remoteJournalPort() const;

Q_SIGNALS:
    void modeChanged(SessionConfig::Mode mode);
    void localJournalPathChanged();
    void remoteJournalUrlChanged();
    void remoteJournalPortChanged();

private:
    void initRemoteJournal();

    Mode mMode{Mode::SYSTEM};
    QString mJournalPath;
    QString mRemoteJournalUrl;
    quint32 mRemoteJournalPort{0};
    std::unique_ptr<SystemdJournalRemote> mRemoteJournal;
};

#endif // SESSIONCONFIG_H

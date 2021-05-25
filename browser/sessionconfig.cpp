/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "sessionconfig.h"
#include "loggingcategories.h"
#include <QFileInfo>

SessionConfig::Mode SessionConfig::mode() const
{
    return mMode;
}

void SessionConfig::setMode(SessionConfig::Mode mode)
{
    if (mode == mMode) {
        return;
    }
    mMode = mode;

    Q_EMIT modeChanged(mode);
}

void SessionConfig::setDisplayUtcTime(bool enforceUtc)
{
    if (enforceUtc == mDisplayUtcTime) {
        return;
    }
    mDisplayUtcTime = enforceUtc;
    Q_EMIT displayUtcTimeChanged(enforceUtc);
}

bool SessionConfig::isDisplayUtcTime() const
{
    return mDisplayUtcTime;
}

void SessionConfig::setLocalJournalPath(const QString &path)
{
    qCDebug(journald) << "Open path" << path;
    // handle QUrl conversion for QML access
    QString resolvedPath = path;
    if (path.startsWith("file://")) {
        resolvedPath.remove(0, 7);
    }

    if (resolvedPath == mJournalPath) {
        return;
    }
    mJournalPath = resolvedPath;
    Q_EMIT localJournalPathChanged();
}

void SessionConfig::setRemoteJournalUrl(const QString &url)
{
    if (url == mRemoteJournalUrl) {
        return;
    }
    mRemoteJournalUrl = url;
    Q_EMIT remoteJournalUrlChanged();
    initRemoteJournal();
}

QString SessionConfig::remoteJournalUrl() const
{
    return mRemoteJournalUrl;
}

void SessionConfig::setRemoteJournalPort(quint32 port)
{
    if (port == mRemoteJournalPort) {
        return;
    }
    mRemoteJournalPort = port;
    Q_EMIT remoteJournalPortChanged();
    initRemoteJournal();
}

quint32 SessionConfig::remoteJournalPort() const
{
    return mRemoteJournalPort;
}

QString SessionConfig::localJournalPath() const
{
    return mJournalPath;
}

void SessionConfig::initRemoteJournal()
{
    if (mRemoteJournalUrl.isEmpty() || mRemoteJournalPort == 0) {
        return;
    }
    mRemoteJournal = std::make_unique<SystemdJournalRemote>(mRemoteJournalUrl, QString::number(mRemoteJournalPort));
    connect(mRemoteJournal.get(), &SystemdJournalRemote::journalFileChanged, [=](){
        setLocalJournalPath(QFileInfo(mRemoteJournal->journalFile()).absolutePath());
    });
}

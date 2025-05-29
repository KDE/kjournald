/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "databaseprovider.h"
#include "kjournaldlib_log_general.h"
#include <QFileInfo>

DatabaseProvider::DatabaseProvider(QObject *parent)
    : QObject(parent)
{
}

DatabaseProvider::~DatabaseProvider() = default;

DatabaseProvider::Mode DatabaseProvider::mode() const
{
    return mMode;
}

void DatabaseProvider::setSystemJournal()
{
    // not setting any path defaults to system journal
    mJournalPath = QString();
    mMode = Mode::SYSTEM;
    Q_EMIT journalPathChanged();
}

QString DatabaseProvider::journalPath() const
{
    return mJournalPath;
}

void DatabaseProvider::setLocalJournalPath(const QString &path)
{
    qCDebug(KJOURNALDLIB_GENERAL) << "Open path" << path;
    // handle QUrl conversion for QML access
    QString resolvedPath = path;
    if (path.startsWith("file://")) {
        resolvedPath.remove(0, 7);
    }

    if (resolvedPath == mJournalPath) {
        return;
    }
    mJournalPath = resolvedPath;
    mMode = Mode::LOCALFOLDER;
    Q_EMIT journalPathChanged();
}

void DatabaseProvider::setRemoteJournalUrl(const QString &url, quint32 port)
{
    if (url == mRemoteJournalUrl && port == mRemoteJournalPort) {
        return;
    }
    mRemoteJournalUrl = url;
    mRemoteJournalPort = port;
    initRemoteJournal();
    mMode = Mode::REMOTE;
    Q_EMIT journalPathChanged();
}

QString DatabaseProvider::remoteJournalUrl() const
{
    return mRemoteJournalUrl;
}

quint32 DatabaseProvider::remoteJournalPort() const
{
    return mRemoteJournalPort;
}

QString DatabaseProvider::localJournalPath() const
{
    return mJournalPath;
}

void DatabaseProvider::initRemoteJournal()
{
    if (mRemoteJournalUrl.isEmpty() || mRemoteJournalPort == 0) {
        return;
    }
    mRemoteJournal = std::make_unique<SystemdJournalRemote>(mRemoteJournalUrl, QString::number(mRemoteJournalPort));
    connect(mRemoteJournal.get(), &SystemdJournalRemote::journalFileChanged, this, [=]() {
        setLocalJournalPath(QFileInfo(mRemoteJournal->journalFile()).absolutePath());
    });
}

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
    setSystemJournal();
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
    mJournalProvider = std::make_shared<LocalJournal>(LocalJournal::Mode::System);
    Q_EMIT journalPathChanged();
}

void DatabaseProvider::setUserJournal()
{
    // not setting any path defaults to system journal
    mJournalPath = QString();
    mMode = Mode::USER;
    mJournalProvider = std::make_shared<LocalJournal>(LocalJournal::Mode::User);
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
    mJournalProvider = std::make_shared<LocalJournal>(mJournalPath);
    Q_EMIT journalPathChanged();
}

void DatabaseProvider::setRemoteJournalUrl(const QString &url, quint32 port)
{
    if (url == mRemoteJournalUrl && port == mRemoteJournalPort) {
        return;
    }
    mRemoteJournalUrl = url;
    mRemoteJournalPort = port;
    initJournal();
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

IJournalProvider *DatabaseProvider::journalProvider()
{
    return mJournalProvider.get();
}

QString DatabaseProvider::localJournalPath() const
{
    return mJournalPath;
}

void DatabaseProvider::initJournal()
{
    if (mRemoteJournalUrl.isEmpty() || mRemoteJournalPort == 0) {
        return;
    }
    auto remoteJournal = std::make_shared<SystemdJournalRemote>(mRemoteJournalUrl, QString::number(mRemoteJournalPort));
    connect(remoteJournal.get(), &SystemdJournalRemote::journalFileChanged, this, [=]() {
        setLocalJournalPath(QFileInfo(remoteJournal->journalFile()).absolutePath());
    });
    mJournalProvider = remoteJournal;
}

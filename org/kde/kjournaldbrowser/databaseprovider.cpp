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
    setLocalJournal();
}

DatabaseProvider::~DatabaseProvider() = default;

DatabaseProvider::Mode DatabaseProvider::mode() const
{
    return mMode;
}

void DatabaseProvider::setLocalJournal()
{
    // not setting any path defaults to system journal
    mJournalPath = QString();
    mMode = Mode::LOCAL_SYSTEM;
    mJournalProvider = std::make_shared<LocalJournal>(LocalJournal::Mode::AnyLocal);
    Q_EMIT journalPathChanged();
}

void DatabaseProvider::restrictToLocalSystemLogs()
{
    if (mMode == Mode::LOCAL_SYSTEM) {
        mJournalProvider = std::make_shared<LocalJournal>(LocalJournal::Mode::LocalSystem);
        Q_EMIT journalPathChanged();
    } else {
        qCWarning(KJOURNALDLIB_GENERAL) << "Cannot restrict non-system logs to machine-id";
    }
}

void DatabaseProvider::restrictToCurrentUserLogs()
{
    if (mMode == Mode::LOCAL_SYSTEM) {
        mJournalProvider = std::make_shared<LocalJournal>(LocalJournal::Mode::LocalSystem);
        Q_EMIT journalPathChanged();
    } else {
        qCWarning(KJOURNALDLIB_GENERAL) << "Cannot restrict non-system logs to current user";
    }
}

QUrl DatabaseProvider::journalPath() const
{
    return QUrl::fromLocalFile(mJournalPath);
}

void DatabaseProvider::setJournalPath(const QUrl &path)
{
    setLocalJournalPath(path.toLocalFile());
}

void DatabaseProvider::setLocalJournalPath(const QString &path)
{
    qCDebug(KJOURNALDLIB_GENERAL) << "Open path" << path;
    if (path == mJournalPath) {
        return;
    }
    mJournalPath = path;
    mMode = Mode::FOLDER;
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

#include "moc_databaseprovider.cpp"

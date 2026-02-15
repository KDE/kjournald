/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "databaseprovider.h"
#include "kjournaldlib_log_general.h"
#include "localjournal.h"
#include "systemdjournalremote.h"
#include <QFileInfo>

DatabaseProvider::DatabaseProvider(QObject *parent)
    : QObject(parent)
{
    setLocalJournal();

    connect(this, &DatabaseProvider::accessChanged, this, &DatabaseProvider::reloadJournal);
    connect(this, &DatabaseProvider::journalPathChanged, this, &DatabaseProvider::journalChanged);
}

DatabaseProvider::~DatabaseProvider() = default;

DatabaseProvider::DatabaseType DatabaseProvider::databaseType() const
{
    return mDatabaseType;
}

DatabaseProvider::DatabaseAccessLimit DatabaseProvider::access() const
{
    return mAccessLimit;
}

void DatabaseProvider::setAccess(DatabaseProvider::DatabaseAccessLimit limit)
{
    if (limit == mAccessLimit) {
        return;
    }
    qCDebug(KJOURNALDLIB_GENERAL) << "change journal access limit to:" << limit;
    mAccessLimit = limit;
    Q_EMIT accessChanged();
}

void DatabaseProvider::setLocalJournal()
{
    // not setting any path defaults to system journal
    mJournalPath = QString();
    mDatabaseType = DatabaseType::LOCAL_SYSTEM;
    mJournalProvider = std::make_shared<LocalJournal>(LocalJournal::Mode::AnyLocal);
    Q_EMIT journalPathChanged();
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
    mDatabaseType = DatabaseType::FOLDER;
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
    mDatabaseType = DatabaseType::REMOTE;
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

void DatabaseProvider::reloadJournal()
{
    if (mDatabaseType == DatabaseType::LOCAL_SYSTEM) {
        switch (mAccessLimit) {
        case DatabaseAccessLimit::ALL:
            mJournalProvider = std::make_shared<LocalJournal>(LocalJournal::Mode::AnyLocal);
            break;
        case DatabaseAccessLimit::CURRENT_USER:
            mJournalProvider = std::make_shared<LocalJournal>(LocalJournal::Mode::CurrentUser);
            break;
        case DatabaseAccessLimit::LOCAL_SYSTEM:
            mJournalProvider = std::make_shared<LocalJournal>(LocalJournal::Mode::LocalSystem);
            break;
        }
        qCDebug(KJOURNALDLIB_GENERAL) << "Reloaded local journal with access" << mAccessLimit;
        Q_EMIT journalChanged();
    } else {
        // nothing to do yet: this handler is only connected to access limit changes
    }
}

#include "moc_databaseprovider.cpp"

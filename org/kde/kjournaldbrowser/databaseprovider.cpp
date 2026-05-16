/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021-2026 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "databaseprovider.h"
#include "kjournaldlib_log_general.h"
#include "localjournal.h"
#include "systemdjournalremote.h"
#include <QFileInfo>

DatabaseProvider::DatabaseProvider(QObject *parent)
    : QObject(parent)
{
    connect(this, &DatabaseProvider::accessChanged, this, &DatabaseProvider::reloadJournal);
    connect(this, &DatabaseProvider::journalChanged, this, &DatabaseProvider::currentJournalInfoTextChanged);
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

void DatabaseProvider::loadSystemJournal()
{
    qCInfo(KJOURNALDLIB_GENERAL) << "Loading local system journal";

    // not setting any path defaults to system journal
    mJournalPath = QString();
    Q_EMIT localJournalPathChanged();

    mDatabaseType = DatabaseType::LOCAL_SYSTEM;
    mJournalProvider = std::make_shared<LocalJournal>(LocalJournal::Mode::AnyLocal);
    Q_EMIT journalChanged();
}

QUrl DatabaseProvider::journalPath() const
{
    return QUrl::fromLocalFile(mJournalPath);
}

void DatabaseProvider::loadJournalFromLocalPath(const QUrl &url)
{
    qCInfo(KJOURNALDLIB_GENERAL) << "Loading journal from:" << url;
    const QString path = url.toString(QUrl::PreferLocalFile);
    qCDebug(KJOURNALDLIB_GENERAL) << "Use decoded path:" << path;
    if (path == mJournalPath) {
        return;
    }
    mJournalPath = path;
    Q_EMIT localJournalPathChanged();

    mDatabaseType = DatabaseType::FOLDER;
    mJournalProvider = std::make_shared<LocalJournal>(mJournalPath);
    Q_EMIT journalChanged();
}

void DatabaseProvider::loadJournalFromRemoteAddress(const QString &url, quint32 port)
{
    if (url == mRemoteJournalUrl && port == mRemoteJournalPort) {
        return;
    }
    mRemoteJournalUrl = url;
    mRemoteJournalPort = port;
    Q_EMIT remoteJournalUrlChanged();
    Q_EMIT remoteJournalPortChanged();

    initJournal();
    mDatabaseType = DatabaseType::REMOTE;
    Q_EMIT journalChanged();
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

QString DatabaseProvider::currentJournalInfoText() const
{
    switch (mDatabaseType) {
    case DatabaseType::FOLDER:
        return QString("%1 [path]").arg(mJournalPath);
    case DatabaseType::LOCAL_SYSTEM:
        return QString("[system]");
    case DatabaseType::REMOTE:
        return QString("%1:%2 [remote]").arg(mRemoteJournalUrl, mRemoteJournalPort);
    }
    return QString();
}

void DatabaseProvider::initJournal()
{
    if (mRemoteJournalUrl.isEmpty() || mRemoteJournalPort == 0) {
        return;
    }
    auto remoteJournal = std::make_shared<SystemdJournalRemote>(mRemoteJournalUrl, QString::number(mRemoteJournalPort));
    connect(remoteJournal.get(), &SystemdJournalRemote::journalFileChanged, this, [=]() {
        mJournalPath = QFileInfo(remoteJournal->journalFile()).absolutePath();
        Q_EMIT localJournalPathChanged();
        mJournalProvider = std::make_shared<LocalJournal>(mJournalPath);
        Q_EMIT journalChanged();
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

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021-2026 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef DATABASEPROVIDER_H
#define DATABASEPROVIDER_H

#include <QObject>
#include <QQmlEngine>
#include <QSettings>
#include <ijournalprovider.h>

/**
 * @brief Provides path to current journald database
 */
class DatabaseProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DatabaseProvider::DatabaseAccessLimit access READ access WRITE setAccess NOTIFY accessChanged FINAL)
    Q_PROPERTY(QUrl journalPath READ journalPath NOTIFY journalPathChanged)

    /**
     * @note this path can either be a directory or a file
     */
    Q_PROPERTY(QString localJournalPath READ localJournalPath NOTIFY journalPathChanged)
    Q_PROPERTY(QString remoteJournalUrl READ remoteJournalUrl NOTIFY journalPathChanged)
    Q_PROPERTY(quint32 remoteJournalPort READ remoteJournalPort NOTIFY journalPathChanged)
    Q_PROPERTY(IJournalProvider *journalProvider READ journalProvider NOTIFY journalChanged)

    QML_ELEMENT
    QML_SINGLETON

public:
    enum class DatabaseType {
        FOLDER, //!< arbitrary folder that is not associated with machine-id / current-user
        LOCAL_SYSTEM, //!< local system journald database
        REMOTE, //!< reading from remote port
    };
    Q_ENUM(DatabaseType);

    /**
     * access limit that is applied to system journal
     *
     * \note these access limits do to make sense when using in combination with FOLDER or REMOTE
     * type logs because neither machine-id are current-user-id are usefull.
     **/
    enum class DatabaseAccessLimit {
        ALL, //!< use all available logs from system journal
        CURRENT_USER, //!< restrict log entries to match current user-id
        LOCAL_SYSTEM, //!< retrict log entries to match local machine-id
    };
    Q_ENUM(DatabaseAccessLimit)

    explicit DatabaseProvider(QObject *parent = nullptr);
    ~DatabaseProvider() override;

    DatabaseProvider::DatabaseType databaseType() const;

    DatabaseProvider::DatabaseAccessLimit access() const;
    void setAccess(DatabaseProvider::DatabaseAccessLimit limit);

    Q_INVOKABLE void setJournalPath(const QUrl &path);
    Q_INVOKABLE void setLocalJournal();
    Q_INVOKABLE void setRemoteJournalUrl(const QString &url, quint32 port);
    Q_INVOKABLE void setLocalJournalPath(const QString &path);

    QUrl journalPath() const;
    QString localJournalPath() const;
    QString remoteJournalUrl() const;
    quint32 remoteJournalPort() const;

    IJournalProvider *journalProvider();

Q_SIGNALS:
    void journalPathChanged();
    void journalChanged();
    void accessChanged();

private:
    void initJournal();
    void reloadJournal();

    DatabaseType mDatabaseType{DatabaseType::LOCAL_SYSTEM};
    DatabaseAccessLimit mAccessLimit{DatabaseAccessLimit::ALL};
    QString mJournalPath;
    QString mRemoteJournalUrl;
    quint32 mRemoteJournalPort{0};

    std::shared_ptr<IJournalProvider> mJournalProvider;
};

#endif

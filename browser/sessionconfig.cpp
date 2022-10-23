/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "sessionconfig.h"
#include "loggingcategories.h"
#include <QFileInfo>

SessionConfig::SessionConfig(QObject *parent)
    : QObject(parent)
{
    mTimeDisplayFormat = mSettings.value("browser/timedisplay").value<TimeDisplay>();
    mFilterCriterium = mSettings.value("browser/filtercriterium").value<FilterCriterium>();
}

SessionConfig::~SessionConfig()
{
    qCDebug(KJOURNALD_DEBUG) << "Sync configuration";
    mSettings.sync();
}

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

void SessionConfig::setTimeDisplay(SessionConfig::TimeDisplay format)
{
    if (format == mTimeDisplayFormat) {
        return;
    }
    mTimeDisplayFormat = format;
    mSettings.setValue("browser/timedisplay", QVariant::fromValue(static_cast<uint8_t>(format)));
    Q_EMIT timeDisplayChanged();
}

SessionConfig::TimeDisplay SessionConfig::timeDisplay() const
{
    return mTimeDisplayFormat;
}

void SessionConfig::setFilterCriterium(FilterCriterium criterium)
{
    if (criterium == mFilterCriterium) {
        return;
    }
    mFilterCriterium = criterium;
    mSettings.setValue("browser/filtercriterium", QVariant::fromValue(static_cast<uint8_t>(criterium)));
    Q_EMIT filterCriteriumChanged();
}

SessionConfig::FilterCriterium SessionConfig::filterCriterium() const
{
    return mFilterCriterium;
}

void SessionConfig::setViewMode(ViewMode mode)
{
    if (mode == mViewMode) {
        return;
    }
    mViewMode = mode;
    Q_EMIT viewModeChanged();
}

SessionConfig::ViewMode SessionConfig::viewMode() const
{
    return mViewMode;
}

void SessionConfig::setLocalJournalPath(const QString &path)
{
    qCDebug(KJOURNALD_DEBUG) << "Open path" << path;
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
    connect(mRemoteJournal.get(), &SystemdJournalRemote::journalFileChanged, [=]() {
        setLocalJournalPath(QFileInfo(mRemoteJournal->journalFile()).absolutePath());
    });
}

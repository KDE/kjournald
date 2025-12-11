/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "systemdjournalremote.h"
#include "kjournaldlib_log_general.h"
#include "systemdjournalremote_p.h"
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QThread>

SystemdJournalRemotePrivate::SystemdJournalRemotePrivate(SystemdJournalRemote *q)
{
    QObject::connect(&mJournalRemoteProcess, &QProcess::errorOccurred, q, &SystemdJournalRemote::handleJournalRemoteProcessErrors);
    mJournalRemoteProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    if (!sanityCheckForSystemdJournalRemoteExec()) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Sanity checks failed, which indidate systemd-journal-remote libexe is not available";
    }
}

bool SystemdJournalRemotePrivate::sanityCheckForSystemdJournalRemoteExec() const
{
    bool result{true};
    if (!QFile::exists(mSystemdJournalRemoteExec)) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Could not find executable:" << mSystemdJournalRemoteExec;
        result = false;
    }

    QFileInfo info(mSystemdJournalRemoteExec);
    if (result && !info.isExecutable()) {
        qCCritical(KJOURNALDLIB_GENERAL) << "systemd-journal-remote not marked as executable:" << mSystemdJournalRemoteExec;
        result = false;
    }

    return result;
}

QString SystemdJournalRemotePrivate::journalFile() const
{
    return mTemporyJournalDir.path() + QLatin1String("/remote.journal");
}

// TODO additional access can easily be implemented by using systemd-journal-remote CLI:
//   --listen-raw=ADDR      Listen for connections at ADDR
//   --listen-http=ADDR     Listen for HTTP connections at ADDR
//   --listen-https=ADDR    Listen for HTTPS connections at ADDR

// TODO add option to persistently safe journal in DB format

// TODO introduce special handling when reaching max system file size due to: https://github.com/systemd/systemd/issues/5242

SystemdJournalRemote::SystemdJournalRemote(const QString &filePath)
    : d(new SystemdJournalRemotePrivate(this))
{
    if (!QFile::exists(filePath)) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Provided export journal file format does not exists, no journal created" << filePath;
    }
    if (!filePath.endsWith(QLatin1String("export"))) {
        qCWarning(KJOURNALDLIB_GENERAL) << "Provided export file has uncommon file ending that is not \".export\":" << filePath;
    }

    // start import
    if (!d->mTemporaryJournalDirWatcher.addPath(d->mTemporyJournalDir.path())) {
        qCWarning(KJOURNALDLIB_GENERAL) << "could not add path to system watcher:" << d->mTemporyJournalDir.path();
    }
    // command structure: systemd-journal-remote --output=foo.journal foo.export
    qCDebug(KJOURNALDLIB_GENERAL) << QLatin1String("starting process: ") + d->mSystemdJournalRemoteExec + QLatin1String(" --output=") + d->journalFile()
            + QLatin1String(" ") + filePath;
    d->mJournalRemoteProcess.start(d->mSystemdJournalRemoteExec, QStringList() << QLatin1String("--output=") + d->journalFile() << filePath);
    // local file access currently only works when the full journal file is written
    d->mJournalRemoteProcess.waitForFinished();
}

void SystemdJournalRemote::handleJournalRemoteProcessErrors(QProcess::ProcessError error)
{
    qCCritical(KJOURNALDLIB_GENERAL) << "systemd-journal-remote error occured:" << error;
}

SystemdJournalRemote::SystemdJournalRemote(const QString &url, const QString &port)
    : d(new SystemdJournalRemotePrivate(this))
{
    if (!(url.startsWith(QLatin1String("https://")) || url.startsWith(QLatin1String("http://")))) {
        qCWarning(KJOURNALDLIB_GENERAL) << "URL seems not begin a valid URL, no http/https prefix:" << url;
    }
    d->mTemporaryJournalDirWatcher.addPath(d->mTemporyJournalDir.path());
    // command structure /lib/systemd/systemd-journal-remote --url http://127.0.0.1 -o /tmp/asdf.journal --split-mode=none
    d->mJournalRemoteProcess.start(d->mSystemdJournalRemoteExec,
                                   QStringList() << QLatin1String("--output=") + d->journalFile() << QLatin1String("--url=") + url + QLatin1Char(':') + port
                                                 << QLatin1String("--split-mode=none"));
    d->mJournalRemoteProcess.waitForStarted();
}

SystemdJournalRemote::~SystemdJournalRemote()
{
    d->mJournalRemoteProcess.terminate();
    d->mJournalRemoteProcess.waitForFinished(1000);
    if (d->mJournalRemoteProcess.state() == QProcess::Running) {
        qCWarning(KJOURNALDLIB_GENERAL) << "Process did not react to SIGTERM in time, sending SIGKILL";
        d->mJournalRemoteProcess.kill();
    }
    d->mJournalRemoteProcess.waitForFinished();
}

QString SystemdJournalRemote::journalFile() const
{
    return d->journalFile();
}

std::unique_ptr<SdJournal> SystemdJournalRemote::openJournal() const
{
    qCDebug(KJOURNALDLIB_GENERAL) << "use temporary journal directory:" << d->mTemporyJournalDir.path();
    // do not test if temporary file already exists, because that can happen delayed
    // after systemd-remote is fully running
    return std::make_unique<SdJournal>(d->mTemporyJournalDir.path(), 0);
}

QString SystemdJournalRemote::currentBootId() const
{
    qCWarning(KJOURNALDLIB_GENERAL) << "Access to remote journal boot ID is not implemented";
    return QString();
}

uint64_t SystemdJournalRemote::usage() const
{
    auto journal = openJournal();
    if (!journal->isValid()) {
        return 0;
    }
    uint64_t size{0};
    int res = sd_journal_get_usage(journal->get(), &size);
    if (res < 0) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Could not obtain journal size:" << strerror(-res);
    }
    return size;
}

bool SystemdJournalRemote::isSystemdRemoteAvailable() const
{
    return d->sanityCheckForSystemdJournalRemoteExec();
}

#include "moc_systemdjournalremote.cpp"

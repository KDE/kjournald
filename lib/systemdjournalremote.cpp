/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "systemdjournalremote.h"
#include "loggingcategories.h"
#include "systemdjournalremote_p.h"
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QThread>

SystemdJournalRemotePrivate::SystemdJournalRemotePrivate() = default;

bool SystemdJournalRemotePrivate::sanityCheckForSystemdJournalRemoveExec() const
{
    bool result{true};
    if (!QFile::exists(mSystemdJournalRemoteExec)) {
        qCCritical(journald) << "Could not find executable:" << mSystemdJournalRemoteExec;
        result = false;
    }

    QFileInfo info(mSystemdJournalRemoteExec);
    if (result && !info.isExecutable()) {
        qCCritical(journald) << "systemd-journal-remote not marked as executable:" << mSystemdJournalRemoteExec;
        result = false;
    }

    return result;
}

QString SystemdJournalRemotePrivate::journalFile() const
{
    return mTemporyJournalDir.path() + "/" + "remote.journal";
}

// TODO additional access can easily be implemented by using systemd-journal-remote CLI:
//   --listen-raw=ADDR      Listen for connections at ADDR
//   --listen-http=ADDR     Listen for HTTP connections at ADDR
//   --listen-https=ADDR    Listen for HTTPS connections at ADDR

// TODO add option to persistently safe journal in DB format

// TODO introduce special handling when reaching max system file size due to: https://github.com/systemd/systemd/issues/5242

SystemdJournalRemote::SystemdJournalRemote(const QString &filePath)
    : d(new SystemdJournalRemotePrivate)
{
    if (!QFile::exists(filePath)) {
        qCCritical(journald()) << "Provided export journal file format does not exists, no journal created" << filePath;
    }
    if (!filePath.endsWith("export")) {
        qCWarning(journald()) << "Provided export file has uncommon file ending that is not \".export\":" << filePath;
    }

    // start import
    d->mTemporaryJournalDirWatcher.addPath(d->mTemporyJournalDir.path());
    d->mJournalRemoteProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    d->sanityCheckForSystemdJournalRemoveExec();
    // command structure: systemd-journal-remote --output=foo.journal foo.export
    d->mJournalRemoteProcess.start(d->mSystemdJournalRemoteExec, QStringList() << "--output=" + d->journalFile() << filePath);
    d->mJournalRemoteProcess.waitForStarted();

    connect(&d->mTemporaryJournalDirWatcher,
            &QFileSystemWatcher::directoryChanged,
            this,
            &SystemdJournalRemote::handleJournalFileCreated,
            Qt::QueuedConnection);
}

void SystemdJournalRemote::handleJournalFileCreated(const QString &path)
{
    qCDebug(journald) << "handleJournaldFileCreated in path:" << path;

    if (path.isEmpty() || !QDir().exists(d->journalFile())) {
        qCCritical(journald) << "Journal directory does not exist, abort opening" << d->journalFile();
        return;
    }

    const char **files = new const char *[1];
    QByteArray journalPath = d->journalFile().toLocal8Bit();
    files[0] = journalPath.data();

    int result = sd_journal_open_files(&d->mJournal, files, 0 /* no flags, directory defines type */);
    if (result < 0) {
        qCCritical(journald) << "Could not open journal:" << strerror(-result);
    }
    delete[] files;

    Q_EMIT journalFileChanged();
}

SystemdJournalRemote::SystemdJournalRemote(const QString &url, const QString &port)
    : d(new SystemdJournalRemotePrivate)
{
    if (!(url.startsWith("https://") || url.startsWith("http://"))) {
        qCWarning(journald()) << "URL seems not begin a valid URL, no http/https prefix:" << url;
    }
    d->mTemporaryJournalDirWatcher.addPath(d->mTemporyJournalDir.path());
    d->mJournalRemoteProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    d->sanityCheckForSystemdJournalRemoveExec();
    // command structure /lib/systemd/systemd-journal-remote --url http://127.0.0.1 -o /tmp/asdf.journal --split-mode=none
    d->mJournalRemoteProcess.start(d->mSystemdJournalRemoteExec,
                                   QStringList() << "--output=" + d->journalFile() << "--url=" + url + ':' + port << "--split-mode=none");
    d->mJournalRemoteProcess.waitForStarted();

    connect(&d->mTemporaryJournalDirWatcher,
            &QFileSystemWatcher::directoryChanged,
            this,
            &SystemdJournalRemote::handleJournalFileCreated,
            Qt::QueuedConnection);
}

SystemdJournalRemote::~SystemdJournalRemote()
{
    d->mJournalRemoteProcess.terminate();
    d->mJournalRemoteProcess.waitForFinished(1000);
    if (d->mJournalRemoteProcess.state() == QProcess::Running) {
        qCWarning(journald()) << "Process did not react to SIGTERM in time, sending SIGKILL";
        d->mJournalRemoteProcess.kill();
    }
    d->mJournalRemoteProcess.waitForFinished();
    sd_journal_close(d->mJournal);
    d->mJournal = nullptr;
}

QString SystemdJournalRemote::journalFile() const
{
    return d->journalFile();
}

sd_journal *SystemdJournalRemote::sdJournal() const
{
    return d->mJournal;
}

bool SystemdJournalRemote::isValid() const
{
    return d->mJournal != nullptr;
}

uint64_t SystemdJournalRemote::usage() const
{
    uint64_t size{0};
    int res = sd_journal_get_usage(d->mJournal, &size);
    if (res < 0) {
        qCCritical(journald) << "Could not obtain journal size:" << strerror(-res);
    }
    return size;
}

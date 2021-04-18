/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "systemdjournalremote.h"
#include "systemdjournalremote_p.h"
#include "loggingcategories.h"
#include <QDir>
#include <QFileInfo>
#include <QProcess>

SystemdJournalRemotePrivate::SystemdJournalRemotePrivate()
    : mJournalFile("XXXXXX.journal")
{
}

bool SystemdJournalRemotePrivate::sanityCheckForSystemdJournalRemoveExec() const
{
    bool result{ true };
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

// TODO additional access can easily be implemented by using systemd-journal-remote CLI:
//   --url=URL              Read events from systemd-journal-gatewayd at URL
//   --listen-raw=ADDR      Listen for connections at ADDR
//   --listen-http=ADDR     Listen for HTTP connections at ADDR
//   --listen-https=ADDR    Listen for HTTPS connections at ADDR

// TODO add option for file split mode, right now all read data is put into single file

// TODO add option to persistently safe journal in DB format

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
    d->mJournalFile.open();
    d->mJournalRemoteProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    d->sanityCheckForSystemdJournalRemoveExec();
    // command structure: systemd-journal-remote --output=foo.journal foo.export
    d->mJournalRemoteProcess.start(d->mSystemdJournalRemoteExec, QStringList() << "--output=" + d->mJournalFile.fileName() << filePath);
    d->mJournalRemoteProcess.waitForStarted();
    d->mJournalRemoteProcess.waitForFinished(); //FIXME enforces full parsing, which is not what we want

    if (!QDir().exists(d->mJournalFile.fileName())) {
        qCCritical(journald) << "Journal directory does not exist, abort opening" << d->mJournalFile.fileName();
        return;
    }

    const char **files = new const char*[1];
    QByteArray journalPath = d->mJournalFile.fileName().toLocal8Bit();
    files[0] = journalPath.data();

    int result = sd_journal_open_files(&d->mJournal, files, 0 /* no flags, directory defines type */);
    if (result < 0) {
        qCCritical(journald) << "Could not open journal:" << strerror(-result);
    }
    delete [] files;
}

SystemdJournalRemote::~SystemdJournalRemote()
{
    // note: since only file based export format is supported at the moment, we can wait
    d->mJournalRemoteProcess.waitForFinished();
    sd_journal_close(d->mJournal);
    d->mJournal = nullptr;
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
    uint64_t size{ 0 };
    int res = sd_journal_get_usage(d->mJournal, &size);
    if (res < 0) {
        qCCritical(journald) << "Could not obtain journal size:" << strerror(-res);
    }
    return size;
}

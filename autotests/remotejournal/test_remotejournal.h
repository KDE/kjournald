/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef TEST_REMOTEJOURNAL_H
#define TEST_REMOTEJOURNAL_H

#include <QObject>

class TestRemoteJournal : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // parser tests
    void exportFormatReaderBasicAccess();
    void exportFormatReaderBinaryMessageAccess();

    void systemdJournalRemoteJournalFromFile();
    void systemdJournalRemoteJournalFromLocalhost();
};
#endif

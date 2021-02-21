/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDUNIQUEQUERYMODEL_P_H
#define JOURNALDUNIQUEQUERYMODEL_P_H

#include <memory>
#include <systemd/sd-journal.h>
#include <QString>
#include <QVector>

class JournaldUniqueQueryModelPrivate
{
public:
    ~JournaldUniqueQueryModelPrivate();
    void closeJournal();
    bool openJournal();
    bool openJournalFromPath(const QString &directory);
    void runQuery();

    sd_journal *mJournal{ nullptr };
    QString mFieldString;
    QVector<std::pair<QString, bool>> mEntries;
};

#endif // JOURNALDUNIQUEQUERYMODEL_P_H

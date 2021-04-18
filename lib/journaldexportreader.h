/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDEXPORTREADER_H
#define JOURNALDEXPORTREADER_H

#include "kjournald_export.h"
#include <QObject>
#include <QTextStream>

class QIODevice;

class KJOURNALD_EXPORT JournaldExportReader : public QObject
{
    Q_OBJECT
public:
    using LogEntry = QHash<QString, QString>;

    JournaldExportReader(QIODevice *device);
    bool atEnd() const;
    bool readNext();
    LogEntry entry() const;

private:
    QIODevice *mDevice{};
    LogEntry mCurrentEntry;
};
#endif

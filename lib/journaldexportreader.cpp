/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journaldexportreader.h"
#include "loggingcategories.h"
#include <QDebug>
#include <QVector>
#include <QtEndian>
#include <climits>

JournaldExportReader::JournaldExportReader(QIODevice *device)
    : mDevice(device)
{
    if (!mDevice || !mDevice->open(QIODevice::ReadOnly)) {
        qCCritical(journald()) << "Could not open device for reading";
        return;
    }
}

bool JournaldExportReader::atEnd() const
{
    return mDevice->atEnd();
}

bool JournaldExportReader::readNext()
{
    if (mDevice->atEnd()) {
        return false;
    }

    // TODO add handling of binary data
    mCurrentEntry.clear();
    while (!mDevice->atEnd()) {
        QString line = mDevice->readLine().trimmed();

        // empty line = beginning of new log entry
        if (line.isEmpty()) {
            break; // found break in log entry
        }

        // if line does not contain "=" then switch to binary reading mode
        int separatorIndex = line.indexOf('=');
        if (separatorIndex > 0) {
            mCurrentEntry[line.left(separatorIndex)] = line.right(line.length() - separatorIndex - 1).trimmed();
        } else {
            qDebug() << "binary mode";
            QString fieldId = line;
            char output[8]; // size stored as uint64_6 = 8 bytes in little endian
            int size = mDevice->read(output, 8);
            // FIXME initial logic to convert value, but result is not correct
            uint64_t binaryBlobSize = le64toh(reinterpret_cast<uint64_t>(output));

            qCWarning(journald()) << "binary messages not yet supported";
            break;
        }
    }

    return true;
}

JournaldExportReader::LogEntry JournaldExportReader::entry() const
{
    return mCurrentEntry;
}

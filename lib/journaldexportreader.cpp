/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journaldexportreader.h"
#include "kjournaldlib_log_general.h"
#include <QDebug>
#include <QVector>
#include <QIODevice>
#include <climits>

JournaldExportReader::JournaldExportReader(QIODevice *device)
    : mDevice(device)
{
    if (!mDevice || !mDevice->open(QIODevice::ReadOnly)) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Could not open device for reading";
        return;
    }
}

bool JournaldExportReader::atEnd() const
{
    return mDevice->atEnd();
}

// Format description: <https://www.freedesktop.org/wiki/Software/systemd/export/>
// - Two journal entries that follow each other are separated by a double newline.
// - Journal fields consisting only of valid non-control UTF-8 codepoints are serialized as they are
//   (i.e. the field name, followed by '=', followed by field data), followed by a newline as
//   separator to the next field. Note that fields containing newlines cannot be formatted like
//   this. Non-control UTF-8 codepoints are the codepoints with value at or above 32 (' '), or
//   equal to 9 (TAB).
// - Other journal fields are serialized in a special binary safe way: field name, followed by
//   newline, followed by a binary 64bit little endian size value, followed by the binary field
//   data, followed by a newline as separator to the next field.
// - Entry metadata that is not actually a field is serialized like it was a field, but beginning
//   with two underscores. More specifically, __CURSOR=, __REALTIME_TIMESTAMP=,
//   __MONOTONIC_TIMESTAMP= are introduced this way. Note that these meta-fields are only generated
//   when actual journal files are serialized. They are omitted for entries that do not originate
//   from a journal file (for example because they are transferred for the first time to be stored
//   in one). Or in other words: if you are generating this format you shouldn't care about these
//   special double-underscore fields. But you might find them usable when you deserialize the
//   format generated by us. Additional fields prefixed with two underscores might be added later
//   on, your parser should skip over the fields it does not know.
// - The order in which fields appear in an entry is undefined and might be different for each entry
//   that is serialized.

bool JournaldExportReader::readNext()
{
    if (mDevice->atEnd()) {
        return false;
    }

    mCurrentEntry.clear();
    while (!mDevice->atEnd()) {
        QString line = QString::fromLocal8Bit(mDevice->readLine().trimmed());
        // empty line = beginning of new log entry
        if (line.isEmpty()) {
            break; // found break in log entry
        }

        // if line does not contain "=" then switch to binary reading mode
        int separatorIndex = line.indexOf(QLatin1Char('='));
        if (separatorIndex > 0) {
            mCurrentEntry[line.left(separatorIndex)] = line.right(line.length() - separatorIndex - 1).trimmed();
        } else {
            QString fieldId = line;
            union {
                uint64_t uint64;
                char chars[4];
            } size;
            qint64 bytes = mDevice->read(size.chars, 8);
            if (bytes != 8) {
                qCWarning(KJOURNALDLIB_GENERAL) << "Journal entry read that has unexpected number of bytes (8 bytes expected)" << bytes;
            }
            mCurrentEntry[fieldId] = QString::fromLocal8Bit(mDevice->read(le64toh(size.uint64)));
            // read line break after binary content, such that reader points to next line
            mDevice->read(1);
        }
    }

    return true;
}

JournaldExportReader::LogEntry JournaldExportReader::entry() const
{
    return mCurrentEntry;
}

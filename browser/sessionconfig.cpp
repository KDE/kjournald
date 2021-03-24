/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "sessionconfig.h"
#include "loggingcategories.h"

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
    emit modeChanged(mode);
}

void SessionConfig::setLocalJournalPath(const QString &path)
{
    qCDebug(journald) << "Open path" << path;
    // handle QUrl conversion for QML access
    QString resolvedPath = path;
    if (path.startsWith("file://")) {
        resolvedPath.remove(0, 7);
    }

    if (resolvedPath == mPath) {
        return;
    }
    mPath = resolvedPath;
    emit localJournalPathChanged();
}

QString SessionConfig::localJournalPath() const
{
    return mPath;
}

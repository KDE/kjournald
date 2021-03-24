/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef SESSIONCONFIG_H
#define SESSIONCONFIG_H

#include <QObject>

/**
 * @brief Central config for current session
 */
class SessionConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SessionConfig::Mode sessionMode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QString localJournalPath READ localJournalPath WRITE setLocalJournalPath NOTIFY localJournalPathChanged)

public:
    enum class Mode {
        LOCALFOLDER, //!< local available journald folder
        SYSTEM //!< local system journald database
    };
    Q_ENUM(Mode);

    void setMode(Mode mode);

    SessionConfig::Mode mode() const;

    void setLocalJournalPath(const QString &path);

    QString localJournalPath() const;

Q_SIGNALS:
    void modeChanged(Mode mode);
    void localJournalPathChanged();

private:
    Mode mMode{Mode::SYSTEM};
    QString mPath;
};

#endif // SESSIONCONFIG_H

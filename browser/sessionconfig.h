/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef SESSIONCONFIG_H
#define SESSIONCONFIG_H

#include "systemdjournalremote.h"
#include <QObject>
#include <QSettings>

/**
 * @brief Central config for current session
 */
class SessionConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SessionConfig::Mode sessionMode READ mode WRITE setMode NOTIFY modeChanged)
    /**
     * @note this path can either be a directory or a file
     */
    Q_PROPERTY(QString localJournalPath READ localJournalPath WRITE setLocalJournalPath NOTIFY localJournalPathChanged)
    Q_PROPERTY(QString remoteJournalUrl READ remoteJournalUrl WRITE setRemoteJournalUrl NOTIFY remoteJournalUrlChanged)
    Q_PROPERTY(quint32 remoteJournalPort READ remoteJournalPort WRITE setRemoteJournalPort NOTIFY remoteJournalPortChanged)
    Q_PROPERTY(SessionConfig::TimeDisplay timeDisplay READ timeDisplay WRITE setTimeDisplay NOTIFY timeDisplayChanged)
    Q_PROPERTY(SessionConfig::FilterCriterium filterCriterium READ filterCriterium WRITE setFilterCriterium NOTIFY filterCriteriumChanged)
    Q_PROPERTY(SessionConfig::ViewMode viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)

public:
    enum class Mode {
        LOCALFOLDER, //!< local available journald folder
        SYSTEM, //!< local system journald database
        REMOTE, //!< reading from remote port
    };
    Q_ENUM(Mode);

    enum class ViewMode {
        BROWSE, //!< interaction with log view leads to browsing
        SELECT, //!< interaction with log view leads to text selection
    };
    Q_ENUM(ViewMode);

    enum class TimeDisplay : uint8_t {
        LOCALTIME, //!< display time as local time
        UTC, //!< display time as UTC time
        MONOTONIC_TIMESTAMP //!< display monotonic timestamp
    };
    Q_ENUM(TimeDisplay);

    enum class FilterCriterium : uint8_t {
        SYSTEMD_UNIT, //!< filter by systemd unit to which the log belongs
        EXECUTABLE, //!< filter by executable name to which the log belogs
    };
    Q_ENUM(FilterCriterium);

    explicit SessionConfig(QObject *parent = nullptr);
    ~SessionConfig() override;

    void setMode(Mode mode);

    SessionConfig::Mode mode() const;

    void setLocalJournalPath(const QString &path);

    QString localJournalPath() const;

    void setRemoteJournalUrl(const QString &url);

    QString remoteJournalUrl() const;

    void setRemoteJournalPort(quint32 port);

    quint32 remoteJournalPort() const;

    void setTimeDisplay(TimeDisplay format);

    TimeDisplay timeDisplay() const;

    void setFilterCriterium(FilterCriterium criterium);

    FilterCriterium filterCriterium() const;

    void setViewMode(ViewMode mode);

    ViewMode viewMode() const;

Q_SIGNALS:
    void modeChanged(Mode mode);
    void localJournalPathChanged();
    void remoteJournalUrlChanged();
    void remoteJournalPortChanged();
    void timeDisplayChanged();
    void filterCriteriumChanged();
    void viewModeChanged();

private:
    void initRemoteJournal();

    Mode mMode{Mode::SYSTEM};
    QString mJournalPath;
    QString mRemoteJournalUrl;
    quint32 mRemoteJournalPort{0};
    TimeDisplay mTimeDisplayFormat{TimeDisplay::UTC};
    FilterCriterium mFilterCriterium{FilterCriterium::SYSTEMD_UNIT};
    ViewMode mViewMode{ViewMode::BROWSE};
    std::unique_ptr<SystemdJournalRemote> mRemoteJournal;
    QSettings mSettings;
};

#endif // SESSIONCONFIG_H

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef BROWSERAPPLICATION_H
#define BROWSERAPPLICATION_H

#include <AbstractKirigamiApplication>
#include <QSettings>

class BrowserApplication : public AbstractKirigamiApplication
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(BrowserApplication::TimeDisplay timeDisplay READ timeDisplay WRITE setTimeDisplay NOTIFY timeDisplayChanged)
    Q_PROPERTY(BrowserApplication::FilterCriterium filterCriterium READ filterCriterium WRITE setFilterCriterium NOTIFY filterCriteriumChanged)
    Q_PROPERTY(BrowserApplication::ViewMode viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)

public:
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

    explicit BrowserApplication(QObject *parent = nullptr);
    ~BrowserApplication() override;

    void setTimeDisplay(TimeDisplay format);

    TimeDisplay timeDisplay() const;

    void setFilterCriterium(FilterCriterium criterium);

    FilterCriterium filterCriterium() const;

    void setViewMode(ViewMode mode);

    ViewMode viewMode() const;

protected:
    void setupActions() override;

Q_SIGNALS:
    void timeDisplayChanged();
    void filterCriteriumChanged();
    void viewModeChanged();

private:
    TimeDisplay mTimeDisplayFormat{TimeDisplay::UTC};
    FilterCriterium mFilterCriterium{FilterCriterium::SYSTEMD_UNIT};
    ViewMode mViewMode{ViewMode::BROWSE};
    QSettings mSettings;
};

#endif // BROWSERAPPLICATION_H

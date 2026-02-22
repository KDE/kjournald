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

    Q_PROPERTY(BrowserApplication::TimeDisplay timeDisplay READ timeDisplay WRITE setTimeDisplay NOTIFY timeDisplayChanged FINAL)
    Q_PROPERTY(BrowserApplication::FilterCriterium filterCriterium READ filterCriterium WRITE setFilterCriterium NOTIFY filterCriteriumChanged FINAL)
    Q_PROPERTY(BrowserApplication::ViewMode viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged FINAL)
    Q_PROPERTY(BrowserApplication::LogViewMode logViewMode READ logViewMode WRITE setLogViewMode NOTIFY logViewModeChanged FINAL)
    Q_PROPERTY(BrowserApplication::ServiceGrouping serviceGrouping READ serviceGrouping WRITE setServiceGrouping NOTIFY serviceGroupingChanged FINAL)
    Q_PROPERTY(quint8 logPriority READ logPriority WRITE setLogPriority NOTIFY logPriorityChanged FINAL)

public:
    enum class ViewMode {
        BROWSE, //!< interaction with log view leads to browsing
        SELECT, //!< interaction with log view leads to text selection
    };
    Q_ENUM(ViewMode)

    enum class LogViewMode {
        ALL_LOGS, //!< display all logs
        ONLY_USER, //!< display only user logs
        ONLY_SYSTEM, //!< display only system logs
    };
    Q_ENUM(LogViewMode)

    enum class TimeDisplay : uint8_t {
        LOCALTIME, //!< display time as local time
        UTC, //!< display time as UTC time
        MONOTONIC_TIMESTAMP //!< display monotonic timestamp
    };
    Q_ENUM(TimeDisplay)

    enum class FilterCriterium : uint8_t {
        SYSTEMD_UNIT, //!< filter by systemd unit to which the log belongs
        EXECUTABLE, //!< filter by executable name to which the log belogs
    };
    Q_ENUM(FilterCriterium)

    enum class ServiceGrouping : uint8_t {
        GROUP_SERVICE_TEMPLATES, //!< display templated service instances by template name
        UNGROUP_SERVICE_TEMPLATES, //!< display templated service instances separately
    };
    Q_ENUM(ServiceGrouping)

    static constexpr QLatin1StringView ID_timeDisplayFormat{"browser/timedisplay"};
    static constexpr QLatin1StringView ID_filterCriterium{"browser/filtercriterium"};
    static constexpr QLatin1StringView ID_serviceGrouping{"browser/servicegrouping"};
    static constexpr QLatin1StringView ID_viewMode{"browser/viewmode"};
    static constexpr QLatin1StringView ID_logViewMode{"browser/logviewmode"};
    static constexpr QLatin1StringView ID_logPriority{"browser/priority"};

    explicit BrowserApplication(QObject *parent = nullptr);
    ~BrowserApplication() override;

    void setTimeDisplay(TimeDisplay format);

    TimeDisplay timeDisplay() const;

    void setFilterCriterium(FilterCriterium criterium);

    FilterCriterium filterCriterium() const;

    void setViewMode(ViewMode mode);

    ViewMode viewMode() const;

    void setLogViewMode(LogViewMode mode);

    LogViewMode logViewMode() const;

    void setServiceGrouping(ServiceGrouping mode);

    ServiceGrouping serviceGrouping() const;

    void setLogPriority(qint8 priority);

    qint8 logPriority() const;

protected:
    void setupActions() override;

Q_SIGNALS:
    void timeDisplayChanged(BrowserApplication::TimeDisplay display);
    void filterCriteriumChanged(BrowserApplication::FilterCriterium criterium);
    void viewModeChanged(BrowserApplication::ViewMode mode);
    void logViewModeChanged(BrowserApplication::LogViewMode mode);
    void serviceGroupingChanged(BrowserApplication::ServiceGrouping mode);
    void logPriorityChanged(quint32 priority);

private:
    TimeDisplay mTimeDisplayFormat{TimeDisplay::UTC};
    FilterCriterium mFilterCriterium{FilterCriterium::SYSTEMD_UNIT};
    ViewMode mViewMode{ViewMode::BROWSE};
    LogViewMode mLogViewMode{LogViewMode::ALL_LOGS};
    ServiceGrouping mServiceGrouping{ServiceGrouping::GROUP_SERVICE_TEMPLATES};
    qint32 mLogPriority{-1};
    QSettings mSettings;
};

#endif // BROWSERAPPLICATION_H

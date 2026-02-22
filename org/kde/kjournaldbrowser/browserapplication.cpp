/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "browserapplication.h"
#include "kjournaldlib_log_general.h"

BrowserApplication::BrowserApplication(QObject *parent)
    : AbstractKirigamiApplication(parent)
{
    if (mSettings.contains(ID_timeDisplayFormat)) {
        mTimeDisplayFormat = mSettings.value(ID_timeDisplayFormat).value<TimeDisplay>();
    }
    if (mSettings.contains(ID_filterCriterium)) {
        mFilterCriterium = mSettings.value(ID_filterCriterium).value<FilterCriterium>();
    }
    if (mSettings.contains(ID_serviceGrouping)) {
        mServiceGrouping = mSettings.value(ID_serviceGrouping).value<ServiceGrouping>();
    }
    if (mSettings.contains(ID_viewMode)) {
        mViewMode = mSettings.value(ID_viewMode).value<ViewMode>();
    }
    if (mSettings.contains(ID_logViewMode)) {
        mLogViewMode = mSettings.value(ID_logViewMode).value<LogViewMode>();
    }
    if (mSettings.contains(ID_logPriority)) {
        mLogPriority = mSettings.value(ID_logPriority).value<quint32>();
    }

    BrowserApplication::setupActions();
}

BrowserApplication::~BrowserApplication()
{
    mSettings.sync();
}

void BrowserApplication::setupActions()
{
    AbstractKirigamiApplication::setupActions();
    // no own actions defined yet
}

void BrowserApplication::setTimeDisplay(BrowserApplication::TimeDisplay format)
{
    if (format == mTimeDisplayFormat) {
        return;
    }
    mTimeDisplayFormat = format;
    qCDebug(KJOURNALDLIB_GENERAL) << "write config key" << ID_timeDisplayFormat << format;
    mSettings.setValue(ID_timeDisplayFormat, QVariant::fromValue(static_cast<uint8_t>(format)));
    Q_EMIT timeDisplayChanged(mTimeDisplayFormat);
}

BrowserApplication::TimeDisplay BrowserApplication::timeDisplay() const
{
    return mTimeDisplayFormat;
}

void BrowserApplication::setFilterCriterium(FilterCriterium criterium)
{
    if (criterium == mFilterCriterium) {
        return;
    }
    mFilterCriterium = criterium;
    qCDebug(KJOURNALDLIB_GENERAL) << "write config key" << ID_filterCriterium << criterium;
    mSettings.setValue(ID_filterCriterium, QVariant::fromValue(static_cast<uint8_t>(criterium)));
    Q_EMIT filterCriteriumChanged(mFilterCriterium);
}

BrowserApplication::FilterCriterium BrowserApplication::filterCriterium() const
{
    return mFilterCriterium;
}

void BrowserApplication::setViewMode(ViewMode mode)
{
    if (mode == mViewMode) {
        return;
    }
    mViewMode = mode;
    qCDebug(KJOURNALDLIB_GENERAL) << "write config key" << ID_viewMode << mViewMode;
    mSettings.setValue(ID_viewMode, QVariant::fromValue(static_cast<uint8_t>(mViewMode)));
    Q_EMIT viewModeChanged(mViewMode);
}

BrowserApplication::ViewMode BrowserApplication::viewMode() const
{
    return mViewMode;
}

void BrowserApplication::setLogViewMode(LogViewMode mode)
{
    if (mode == mLogViewMode) {
        return;
    }
    mLogViewMode = mode;
    qCDebug(KJOURNALDLIB_GENERAL) << "write config key" << ID_logViewMode << mode;
    mSettings.setValue(ID_logViewMode, QVariant::fromValue(static_cast<uint8_t>(mode)));
    Q_EMIT logViewModeChanged(mLogViewMode);
}

BrowserApplication::LogViewMode BrowserApplication::logViewMode() const
{
    return mLogViewMode;
}

void BrowserApplication::setServiceGrouping(ServiceGrouping mode)
{
    if (mode == mServiceGrouping) {
        return;
    }
    mServiceGrouping = mode;
    qCDebug(KJOURNALDLIB_GENERAL) << "write config key" << ID_serviceGrouping << mode;
    mSettings.setValue(ID_serviceGrouping, QVariant::fromValue(static_cast<uint8_t>(mode)));
    Q_EMIT serviceGroupingChanged(mServiceGrouping);
}

BrowserApplication::ServiceGrouping BrowserApplication::serviceGrouping() const
{
    return mServiceGrouping;
}

void BrowserApplication::setLogPriority(qint8 priority)
{
    if (mLogPriority != priority) {
        qCDebug(KJOURNALDLIB_GENERAL) << "write config key" << ID_logPriority << priority;
        mLogPriority = priority;
        mSettings.setValue(ID_logPriority, QVariant::fromValue(priority));
        Q_EMIT logPriorityChanged(priority);
    }
}

qint8 BrowserApplication::logPriority() const
{
    return mLogPriority;
}

#include "moc_browserapplication.cpp"

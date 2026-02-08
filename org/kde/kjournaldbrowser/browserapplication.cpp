/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "browserapplication.h"

BrowserApplication::BrowserApplication(QObject *parent)
    : AbstractKirigamiApplication(parent)
{
    mTimeDisplayFormat = mSettings.value(ID_timeDisplayFormat).value<TimeDisplay>();
    mFilterCriterium = mSettings.value(ID_filterCriterium).value<FilterCriterium>();
    mServiceGrouping = mSettings.value(ID_serviceGrouping).value<ServiceGrouping>();
    mViewMode = mSettings.value(ID_viewMode).value<ViewMode>();
    mLogViewMode = mSettings.value(ID_logViewMode).value<LogViewMode>();

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
    mSettings.setValue(ID_serviceGrouping, QVariant::fromValue(static_cast<uint8_t>(mode)));
    Q_EMIT serviceGroupingChanged(mServiceGrouping);
}

BrowserApplication::ServiceGrouping BrowserApplication::serviceGrouping() const
{
    return mServiceGrouping;
}

#include "moc_browserapplication.cpp"

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "browserapplication.h"

BrowserApplication::BrowserApplication(QObject *parent)
    : AbstractKirigamiApplication(parent)
{
    mTimeDisplayFormat = mSettings.value("browser/timedisplay").value<TimeDisplay>();
    mFilterCriterium = mSettings.value("browser/filtercriterium").value<FilterCriterium>();

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
    mSettings.setValue("browser/timedisplay", QVariant::fromValue(static_cast<uint8_t>(format)));
    Q_EMIT timeDisplayChanged();
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
    mSettings.setValue("browser/filtercriterium", QVariant::fromValue(static_cast<uint8_t>(criterium)));
    Q_EMIT filterCriteriumChanged();
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
    Q_EMIT viewModeChanged();
}

BrowserApplication::ViewMode BrowserApplication::viewMode() const
{
    return mViewMode;
}

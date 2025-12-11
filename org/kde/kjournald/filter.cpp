/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2024 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "filter.h"

std::optional<quint8> Filter::priorityFilter() const
{
    return mPriority;
}

int Filter::priorityFilterInt() const
{
    return mPriority.value_or(0);
}

void Filter::setPriorityFilter(int priority)
{
    if (priority >= 0) {
        mPriority = priority;
    } else {
        mPriority = std::nullopt;
    }
}

void Filter::resetPriorityFilter()
{
    mPriority.reset();
}

QStringList Filter::bootFilter() const
{
    return mBootFilter;
}

void Filter::setBootFilter(const QStringList &boots)
{
    mBootFilter = boots;
}

QStringList Filter::systemdUnitFilter() const
{
    return mUnitFilter;
}

void Filter::setSystemdUnitFilter(const QStringList &units)
{
    mUnitFilter = units;
}

QStringList Filter::exeFilter() const
{
    return mExeFilter;
}

void Filter::setExeFilter(const QStringList &exe)
{
    mExeFilter = exe;
}

bool Filter::areKernelMessagesEnabled() const
{
    return mEnableKernelMessages;
}

void Filter::setKernelMessagesEnabled(bool enabled)
{
    mEnableKernelMessages = enabled;
}

QDebug operator<<(QDebug debug, const Filter &c)
{
    debug.nospace() << "filter(priority: " << c.priorityFilterInt() << ", boot: " << c.bootFilter() << ", exe: " << c.exeFilter()
                    << ", unit: " << c.systemdUnitFilter() << ", kernel: " << c.areKernelMessagesEnabled() << ")";
    return debug.space();
}

#include "moc_filter.cpp"

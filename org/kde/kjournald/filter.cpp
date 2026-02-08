/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2024-2026 Andreas Cord-Landwehr <cordlandwehr@kde.org>
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

QStringList Filter::systemdUserUnitFilter() const
{
    return mUserUnitFilter;
}

void Filter::setSystemdUserUnitFilter(const QStringList &units)
{
    mUserUnitFilter = units;
}

QStringList Filter::systemdSystemUnitFilter() const
{
    return mSystemUnitFilter;
}

void Filter::setSystemdSystemUnitFilter(const QStringList &units)
{
    mSystemUnitFilter = units;
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
                    << ", user-unit: " << c.systemdUserUnitFilter() << ", system-unit: " << c.systemdSystemUnitFilter()
                    << ", kernel: " << c.areKernelMessagesEnabled() << ")";
    return debug.space();
}

#include "moc_filter.cpp"

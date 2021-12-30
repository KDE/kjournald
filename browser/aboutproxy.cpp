/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "aboutproxy.h"

KAboutData AboutProxy::aboutData() const
{
    return KAboutData::applicationData();
}

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "browserapplication.h"

BrowserApplication::BrowserApplication(QObject *parent)
    : AbstractKirigamiApplication(parent)
{
    BrowserApplication::setupActions();
}

void BrowserApplication::setupActions()
{
    AbstractKirigamiApplication::setupActions();
    // no own actions defined yet
}

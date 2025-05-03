/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef BROWSERAPPLICATION_H
#define BROWSERAPPLICATION_H

#include <AbstractKirigamiApplication>

class BrowserApplication : public AbstractKirigamiApplication
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    BrowserApplication(QObject *parent = nullptr);

protected:
    void setupActions() override;
};

#endif // BROWSERAPPLICATION_H

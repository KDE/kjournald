/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef CLIPBOARDPROXY_H
#define CLIPBOARDPROXY_H

#include <QObject>

class ClipboardProxy : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardProxy(QObject *parent = nullptr);
    Q_INVOKABLE void setText(const QString &text);
};

#endif // CLIPBOARDPROXY_H

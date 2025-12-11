/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "clipboardproxy.h"
#include <QClipboard>
#include <QGuiApplication>

ClipboardProxy::ClipboardProxy(QObject *parent)
    : QObject(parent)
{
}

void ClipboardProxy::setText(const QString &text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(text);
    }
}

#include "moc_clipboardproxy.cpp"

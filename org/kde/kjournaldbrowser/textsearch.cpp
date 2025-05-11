/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "textsearch.h"

TextSearch::TextSearch(QObject *parent)
    : QObject{parent}
{
}

QString TextSearch::needle() const
{
    return m_needle;
}

void TextSearch::setNeedle(const QString &needle)
{
    if (m_needle == needle) {
        return;
    }
    m_needle = needle;
    Q_EMIT needleChanged();
}

bool TextSearch::isHighlightMode() const
{
    return m_hightlightMode;
}

void TextSearch::setHighlightMode(bool highlightMode)
{
    if (m_hightlightMode == highlightMode) {
        return;
    }
    m_hightlightMode = highlightMode;
    Q_EMIT highlightModeChanged();
}

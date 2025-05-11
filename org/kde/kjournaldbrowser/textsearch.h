/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef TEXTSEARCH_H
#define TEXTSEARCH_H

#include <QObject>
#include <QQmlEngine>

class TextSearch : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString needle READ needle WRITE setNeedle NOTIFY needleChanged FINAL)
    Q_PROPERTY(bool highlightMode READ isHighlightMode WRITE setHighlightMode NOTIFY highlightModeChanged FINAL)
    Q_PROPERTY(bool caseSensitive READ isCaseSensitive WRITE setCaseSensitive NOTIFY caseSensitiveChanged FINAL)

    QML_ELEMENT
    QML_SINGLETON

public:
    explicit TextSearch(QObject *parent = nullptr);
    QString needle() const;
    void setNeedle(const QString &needle);
    bool isHighlightMode() const;
    void setHighlightMode(bool highlightMode);
    bool isCaseSensitive() const;
    void setCaseSensitive(bool caseSensitive);

Q_SIGNALS:
    void needleChanged();
    void highlightModeChanged();
    void caseSensitiveChanged();

private:
    QString m_needle;
    bool m_hightlightMode{false};
    Qt::CaseSensitivity m_caseSensitive = Qt::CaseSensitivity::CaseInsensitive;
};

#endif // TEXTSEARCH_H

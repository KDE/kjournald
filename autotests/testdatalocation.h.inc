/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef TESDATALOCATION_H
#define TESDATALOCATION_H

#include <QLatin1StringView>

static constexpr QLatin1StringView JOURNAL_LOCATION("@CMAKE_CURRENT_BINARY_DIR@/journal/");
static constexpr QLatin1StringView JOURNAL_EXPORT_FORMAT_EXAMPLE("@CMAKE_CURRENT_BINARY_DIR@/journalexportformat_example.export");
static constexpr QLatin1StringView JOURNAL_EXPORT_FORMAT_BINARY_EXAMPLE("@CMAKE_CURRENT_BINARY_DIR@/journalexportformat_binary_example.export");

#endif

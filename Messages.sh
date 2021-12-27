#!/bin/sh -x
# SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
# SPDX-License-Identifier: CC0-1.0

$XGETTEXT $(find . -name \*.qml -or -name \*.cpp) -o $podir/kjournald.pot

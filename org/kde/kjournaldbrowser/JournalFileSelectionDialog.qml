/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021-2026 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick
import QtQuick.Dialogs
import org.kde.kjournaldbrowser
import org.kde.ki18n

pragma ComponentBehavior: Bound

Item {
    id: root
    function open() {
        fileDialog.open()
    }

    FileDialog {
        id: fileDialog
        title: KI18n.i18nc("@title", "Select journal file")
        nameFilters: [KI18n.i18nc("@item", "Journal files (*.journal)"), KI18n.i18nc("@item", "All files (*)")]
        onAccepted: {
            DatabaseProvider.loadJournalFromLocalPath(fileDialog.selectedFile)
        }
    }
}

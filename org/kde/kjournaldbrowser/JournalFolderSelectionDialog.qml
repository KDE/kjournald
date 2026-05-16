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
    function open(path: string) {
        if (path !== 'undefined' && path !== "") {
            if (path.startsWith("file://")) {
                folderDialog.currentFolder = path
            } else {
                folderDialog.currentFolder = "file://" + path
            }
        } else {
            folderDialog.currentFolder = "file://" + DatabaseProvider.localJournalPath
        }
        console.log(`open folder dialog with current path ${folderDialog.currentFolder}`)

        folderDialog.open()
    }

    FolderDialog {
        id: folderDialog
        title: KI18n.i18nc("@title", "Select journal folder")
        onAccepted: {
            DatabaseProvider.loadJournalFromLocalPath(folderDialog.selectedFolder)
        }
    }
}
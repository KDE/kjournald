# KJournald

This project aims to provide an abstraction of the systemd's journald API in terms of QAbstractItemModel classes. The main purpose is to ease the integration of journald into Qt based applications (both QML and QtWidget).

Additional to the library, the project provides a reference implementation of the API, called `kjournaldbrowser`. Even though that application provides a powerful journal database reader, we aim to do a clear split between reuseable library and application logic.

#### Screenshot

![](https://cdn.kde.org/screenshots/kjournald/kjournald.png)

## License
All source files in the repository are supposed to be compatible with LGPL-2.1-or-later in order to easily move them between application and library part.

- SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
- SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>

## Library Features
- abstraction for unique query API, which can obtain unique values like list of all boot IDs, list of all units.
- QAbstractItemModel for list of boots
- QAbstractItemModel for full journald DB with filter options per boot and unit

### Library Usage

In your CMakeLists.txt, add:

```
find_package(KJournald 0.0.2)
target_link_libraries(mycoolapp
    PUBLIC KJournald)
```

## kjournaldbrowser Features
The browser provides a single window overview that can be configured to show a desired combination of systemd units and their prioritized messages.

- the browser can read journald DB entries via the following ways:
    - direct access to a local journald DB folder
- UI filters:
    - filter by systemd_unit (also via rainbow colors)
    - filter by boot
    - filter by priority (also via rainbow colors)
    - filter by kernel messages or not
- additional log information:
    - time available as UTC
- browsing
    - copy current view (CTRL+C)
    - key-based navigation
        - scroll forward: PAGE_DOWN
        - scroll backward: PAGE_UP
        - scroll to most recent entry: CTRL+PAGE_DOWN
        - scroll to oldest entry: CTRL+PAGE_UP
    - highlight search strings
- use "kjournaldbrowser -D <path>" to directly open specific database folder

## Library Dependencies
- Qt::Core, Qt::Quick
- systemd

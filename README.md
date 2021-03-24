# KJournald

This project aims to provide an abstraction of the systemd's journald API in terms of QAbstractItemModel classes. The main purpose is to ease the integration of journald into Qt based applications (both QML and QtWidget).

Additional to the library, the project provides a reference implementation of the API, called "journald-browser" which might provide value of itself. The focus though is the library's API.

## Library Features
- abstraction for unique query API, which can obtain unique values like list of all boot IDs, list of all units.
- QAbstractItemModel for list of boots
- QAbstractItemModel for full journald DB with filter options per boot and unit

## journald-browser Features
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
    - key-based navigation (PAGE_DOWN, PAGE_UP)
    - highlight search strings

## Dependencies
- Qt::Core, Qt::Quick
- systemd
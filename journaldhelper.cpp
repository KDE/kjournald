#include "journaldhelper.h"
#include <QDebug>

JournaldHelper::JournaldHelper()
{
    int result;
    result = sd_journal_open(&mJournal, SD_JOURNAL_LOCAL_ONLY);
    if (result < 0) {
        qCritical() << "Failed to open journal:" << strerror(-result);
    }
}

QVector<QString> JournaldHelper::queryUnique(Field field) const
{
    QVector<QString> dataList;
    const void *data;
    size_t length;
    int result;

    std::string fieldString;
    switch(field) {
    case Field::_BOOT_ID:
        fieldString = "_BOOT_ID";
        break;
    case Field::_SYSTEMD_CGROUP:
        fieldString = "_SYSTEMD_CGROUP";
        break;
    case Field::_SYSTEMD_OWNER_UID:
        fieldString = "_SYSTEMD_OWNER_UID";
        break;
    case Field::_SYSTEMD_SESSION:
        fieldString = "_SYSTEMD_SESSION";
        break;
    case Field::_SYSTEMD_SLICE:
        fieldString = "_SYSTEMD_SLICE";
        break;
    case Field::_SYSTEMD_UNIT:
        fieldString = "_SYSTEMD_UNIT";
        break;
    case Field::_SYSTEMD_USER_SLICE:
        fieldString = "_SYSTEMD_USER_SLICE";
        break;
    case Field::_SYSTEMD_USER_UNIT:
        fieldString = "_SYSTEMD_USER_UNIT";
        break;
    }

    result = sd_journal_query_unique(mJournal, fieldString.c_str());
    if (result < 0) {
        qCritical() << "Failed to query journal:" << strerror(-result);
        return dataList;
    }
    const int fieldLength = fieldString.length() + 1;
    SD_JOURNAL_FOREACH_UNIQUE(mJournal, data, length) {
        QString dataStr = static_cast<const char*>(data);
        dataList << dataStr.remove(0, fieldLength);
    }
    return dataList;
}

JournaldHelper::~JournaldHelper()
{
    sd_journal_close(mJournal);
}

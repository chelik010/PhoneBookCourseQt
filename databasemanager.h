#pragma once

#include <QSqlDatabase>
#include <vector>
#include "Contact.h"

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool open();
    bool ensureSchema();

    std::vector<Contact> loadAll();
    bool insertContact(const Contact& c, int* outId = nullptr);
    bool updateContact(int contactId, const Contact& c);
    bool deleteContact(int contactId);

    QSqlDatabase& db();

private:
    DatabaseManager();
    QSqlDatabase m_db;
};

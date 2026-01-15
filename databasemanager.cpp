#include "databasemanager.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager() {
    // Важно: addDatabase после того, как уже есть QApplication (у тебя это ок сейчас)
    m_db = QSqlDatabase::addDatabase("QPSQL", "phonebook_conn");
    m_db.setHostName("127.0.0.1");
    m_db.setPort(5433);
    m_db.setDatabaseName("phonebook_db");
    m_db.setUserName("postgres");
    m_db.setPassword("chelik001A");
}

bool DatabaseManager::open() {
    if (m_db.isOpen()) return true;
    if (!m_db.open()) {
        qDebug() << "Database open error:" << m_db.lastError().text();
        return false;
    }
    return true;
}

QSqlDatabase& DatabaseManager::db() { return m_db; }

bool DatabaseManager::ensureSchema() {
    QSqlQuery q(m_db);

    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS contacts(
            id SERIAL PRIMARY KEY,
            last_name   TEXT NOT NULL,
            first_name  TEXT NOT NULL,
            middle_name TEXT,
            address     TEXT,
            birth_date  DATE,
            email       TEXT NOT NULL UNIQUE
        );
    )")) { qDebug() << q.lastError().text(); return false; }

    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS phones(
            id SERIAL PRIMARY KEY,
            contact_id INTEGER NOT NULL REFERENCES contacts(id) ON DELETE CASCADE,
            number TEXT NOT NULL,
            type   TEXT NOT NULL
        );
    )")) { qDebug() << q.lastError().text(); return false; }

    return true;
}

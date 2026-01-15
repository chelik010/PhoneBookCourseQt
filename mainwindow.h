#pragma once

#include <QMainWindow>
#include <QString>
#include <vector>

#include "ContactBook.h"
#include "Validator.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool importFromFileToDbIfEmpty();

    Ui::MainWindow *ui;

    ContactBook m_book;
    QString     m_dataFile;

    // ====== Режимы хранения ======
    bool m_useDb = false;                 // true = PostgreSQL, false = файл
    std::vector<int> m_contactDbIds;      // index в m_book.contacts() -> contact_id в БД

    // для поиска/фильтрации
    QString m_lastFilter;
    std::vector<std::size_t> m_rowToIndex;   // row -> index в m_book.contacts()

    // ====== File ======
    void loadContactsFromFile();
    void saveContactsToFile();

    // ====== DB ======
    bool ensureDbSchema();            // CREATE TABLE IF NOT EXISTS ...
    bool loadContactsFromDb();        // SELECT + сбор ContactBook
    bool insertContactToDb(const Contact &c);
    bool updateContactInDb(int contactId, const Contact &c);
    bool deleteContactFromDb(int contactId);

    void loadContacts();              // выбирает DB/File
    void saveContacts();              // сохраняет только для File-режима
    void refreshTable(const QString &filter = QString());

private slots:
    void on_btnAdd_clicked();
    void on_btnEdit_clicked();
    void on_btnDelete_clicked();
    void on_btnSearch_clicked();
    void on_btnSort_clicked();   // ← ВАЖНО, чтобы было ТОЧНО так
};

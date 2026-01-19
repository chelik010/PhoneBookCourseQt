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

    bool m_useDb = false;
    std::vector<int> m_contactDbIds;

    QString m_lastFilter;
    std::vector<std::size_t> m_rowToIndex;

    void loadContactsFromFile();
    void saveContactsToFile();

    bool ensureDbSchema();
    bool loadContactsFromDb();
    bool insertContactToDb(const Contact &c);
    bool updateContactInDb(int contactId, const Contact &c);
    bool deleteContactFromDb(int contactId);

    void loadContacts();
    void saveContacts();
    void refreshTable(const QString &filter = QString());

private slots:
    void on_btnAdd_clicked();
    void on_btnEdit_clicked();
    void on_btnDelete_clicked();
    void on_btnSearch_clicked();
    void on_btnSort_clicked();
};

#include "mainwindow.h"
#include "ui_mainwindow.h"
// TASK3: PostgreSQL storage enabled
#include <QCoreApplication>
#include <QTableWidgetItem>
#include <QString>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateEdit>
#include <QDate>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QVariant>
#include <QDebug>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>


// ==================== Диалог ввода/редактирования контакта ====================

class ContactDialog : public QDialog
{
public:
    explicit ContactDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(tr("Контакт"));

        m_lastNameEdit   = new QLineEdit(this);
        m_firstNameEdit  = new QLineEdit(this);
        m_middleNameEdit = new QLineEdit(this);
        m_addressEdit    = new QLineEdit(this);
        m_birthDateEdit  = new QDateEdit(this);
        m_emailEdit      = new QLineEdit(this);

        // ---- Телефоны ----
        m_phoneEdit    = new QLineEdit(this);
        m_phoneTypeBox = new QComboBox(this);
        m_phoneList    = new QListWidget(this);
        m_addPhoneBtn  = new QPushButton(tr("Добавить"), this);
        m_delPhoneBtn  = new QPushButton(tr("Удалить выбранный"), this);

        // Настройки даты
        m_birthDateEdit->setDisplayFormat("yyyy-MM-dd");
        m_birthDateEdit->setCalendarPopup(true);
        m_birthDateEdit->setDate(QDate(2000, 1, 1));

        m_phoneEdit->setPlaceholderText("+78121234567");
        m_emailEdit->setPlaceholderText("user1@domain1");

        m_phoneTypeBox->addItem("mobile");
        m_phoneTypeBox->addItem("home");
        m_phoneTypeBox->addItem("work");
        m_phoneTypeBox->addItem("other");

        // UI для добавления телефона: [номер] [тип] [кнопка]
        auto *phoneRow = new QHBoxLayout;
        phoneRow->addWidget(m_phoneEdit);
        phoneRow->addWidget(m_phoneTypeBox);
        phoneRow->addWidget(m_addPhoneBtn);

        auto *form = new QFormLayout;
        form->addRow(tr("Фамилия*"),      m_lastNameEdit);
        form->addRow(tr("Имя*"),          m_firstNameEdit);
        form->addRow(tr("Отчество"),      m_middleNameEdit);
        form->addRow(tr("Адрес"),         m_addressEdit);
        form->addRow(tr("Дата рождения"), m_birthDateEdit);
        form->addRow(tr("E-mail*"),       m_emailEdit);

        form->addRow(tr("Телефон*"),      phoneRow);        // добавление в список
        form->addRow(tr("Все телефоны"),  m_phoneList);     // список телефонов
        form->addRow(QString(),           m_delPhoneBtn);   // кнопка удаления

        auto *buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

        connect(buttons, &QDialogButtonBox::accepted,
                this, &ContactDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected,
                this, &ContactDialog::reject);

        connect(m_addPhoneBtn, &QPushButton::clicked, this, [this]() {
            addPhoneFromInputs();
        });
        connect(m_delPhoneBtn, &QPushButton::clicked, this, [this]() {
            deleteSelectedPhone();
        });

        auto *layout = new QVBoxLayout;
        layout->addLayout(form);
        layout->addWidget(buttons);
        setLayout(layout);
    }

    // заполнение существующим контактом (для редактирования)
    void setContact(const Contact &c)
    {
        m_lastNameEdit->setText(QString::fromStdString(c.lastName()));
        m_firstNameEdit->setText(QString::fromStdString(c.firstName()));
        m_middleNameEdit->setText(QString::fromStdString(c.middleName()));
        m_addressEdit->setText(QString::fromStdString(c.address()));

        const std::string ds = c.birthDate().toString();
        QDate qd = QDate::fromString(QString::fromStdString(ds), "yyyy-MM-dd");
        if (!qd.isValid())
            qd = QDate(2000, 1, 1);
        m_birthDateEdit->setDate(qd);

        m_emailEdit->setText(QString::fromStdString(c.email()));

        m_phoneList->clear();
        const auto &phones = c.phones();
        for (const auto &ph : phones) {
            addPhoneToList(QString::fromStdString(ph.number()),
                           QString::fromStdString(PhoneNumber::typeToString(ph.type())));
        }

        // Для удобства: подставим первый телефон в поля ввода
        if (!phones.empty()) {
            m_phoneEdit->setText(QString::fromStdString(phones[0].number()));
            QString typeStr = QString::fromStdString(PhoneNumber::typeToString(phones[0].type()));
            int idx = m_phoneTypeBox->findText(typeStr);
            if (idx >= 0) m_phoneTypeBox->setCurrentIndex(idx);
        }
    }

    Contact contact() const { return m_contact; }

protected:
    void accept() override
    {
        QString ln  = m_lastNameEdit->text().trimmed();
        QString fn  = m_firstNameEdit->text().trimmed();
        QString mn  = m_middleNameEdit->text().trimmed();
        QString adr = m_addressEdit->text().trimmed();
        QString em  = m_emailEdit->text().trimmed();

        // Если пользователь набрал телефон в поле, но не нажал "Добавить" — добавим автоматически
        if (!m_phoneEdit->text().trimmed().isEmpty()) {
            addPhoneFromInputs(/*silent*/true);
        }

        // Проверки по требованиям
        if (!Validator::isValidName(ln.toStdString()) ||
            !Validator::isValidName(fn.toStdString()))
        {
            QMessageBox::warning(this, tr("Ошибка"),
                                 tr("Фамилия и Имя должны быть корректными."));
            return;
        }

        if (!mn.isEmpty() && !Validator::isValidName(mn.toStdString()))
        {
            QMessageBox::warning(this, tr("Ошибка"),
                                 tr("Отчество введено некорректно."));
            return;
        }

        if (!Validator::isValidEmail(em.toStdString()))
        {
            QMessageBox::warning(this, tr("Ошибка"),
                                 tr("E-mail введён некорректно."));
            return;
        }

        QDate qd = m_birthDateEdit->date();
        Date d = Date::fromString(qd.toString("yyyy-MM-dd").toStdString());
        if (!d.isValid() || !Validator::isValidBirthDate(d))
        {
            QMessageBox::warning(this, tr("Ошибка"),
                                 tr("Дата рождения некорректна."));
            return;
        }

        // Главное: хотя бы один телефон
        if (m_phoneList->count() == 0) {
            QMessageBox::warning(this, tr("Ошибка"),
                                 tr("Нужно указать хотя бы один телефон."));
            return;
        }

        // Собираем Contact
        m_contact = Contact(
            ln.toStdString(),
            fn.toStdString(),
            mn.toStdString(),
            adr.toStdString(),
            d,
            em.toStdString()
            );

        // Добавляем все телефоны из списка
        for (int i = 0; i < m_phoneList->count(); ++i) {
            QListWidgetItem *it = m_phoneList->item(i);
            QString num = it->data(Qt::UserRole).toString();
            QString typ = it->data(Qt::UserRole + 1).toString();

            PhoneType t = PhoneNumber::stringToType(typ.toStdString());
            m_contact.addPhone(PhoneNumber(num.toStdString(), t));
        }

        QDialog::accept();
    }

private:
    void addPhoneFromInputs(bool silent = false)
    {
        QString ph = m_phoneEdit->text().trimmed();
        if (ph.isEmpty())
            return;

        if (!Validator::isValidPhone(ph.toStdString()))
        {
            if (!silent) {
                QMessageBox::warning(this, tr("Ошибка"),
                                     tr("Телефон введён в неверном формате."));
            }
            return;
        }

        QString pt = m_phoneTypeBox->currentText();

        // Можно запретить дубликаты (номер+тип)
        for (int i = 0; i < m_phoneList->count(); ++i) {
            auto *it = m_phoneList->item(i);
            if (it->data(Qt::UserRole).toString() == ph &&
                it->data(Qt::UserRole + 1).toString() == pt)
            {
                if (!silent) {
                    QMessageBox::information(this, tr("Телефон"),
                                             tr("Такой телефон уже добавлен."));
                }
                m_phoneEdit->clear();
                return;
            }
        }

        addPhoneToList(ph, pt);
        m_phoneEdit->clear();
    }

    void addPhoneToList(const QString &number, const QString &type)
    {
        // Текст как будет показан в списке
        auto *item = new QListWidgetItem(QString("%1: %2").arg(type, number));
        item->setData(Qt::UserRole, number);
        item->setData(Qt::UserRole + 1, type);
        m_phoneList->addItem(item);
    }

    void deleteSelectedPhone()
    {
        auto *it = m_phoneList->currentItem();
        if (!it) return;
        delete it;
    }

private:
    QLineEdit *m_lastNameEdit   = nullptr;
    QLineEdit *m_firstNameEdit  = nullptr;
    QLineEdit *m_middleNameEdit = nullptr;
    QLineEdit *m_addressEdit    = nullptr;
    QDateEdit *m_birthDateEdit  = nullptr;
    QLineEdit *m_emailEdit      = nullptr;

    QLineEdit   *m_phoneEdit    = nullptr;
    QComboBox   *m_phoneTypeBox = nullptr;
    QListWidget *m_phoneList    = nullptr;
    QPushButton *m_addPhoneBtn  = nullptr;
    QPushButton *m_delPhoneBtn  = nullptr;

    Contact m_contact;
};

// =========================== MainWindow ===========================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_dataFile(QCoreApplication::applicationDirPath() + "/contacts.txt")
{
    ui->setupUi(this);

    // --- Пытаемся включить DB-режим ---
    qDebug() << "SQL drivers:" << QSqlDatabase::drivers();

    // ВАЖНО: даём имя коннекта, чтобы не плодить default-connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "phonebook_conn");
    db.setHostName("127.0.0.1");
    db.setPort(5433);
    db.setDatabaseName("phonebook_db");
    db.setUserName("postgres");
    db.setPassword("chelik001A");

    if (!db.open()) {
        m_useDb = false;
        qDebug() << "DB OPEN FAILED:" << db.lastError().text();
    } else {
        qDebug() << "DB OPEN OK";
        m_useDb = ensureDbSchema();
        if (m_useDb) {
            importFromFileToDbIfEmpty();
        }
        if (!m_useDb) {
            qDebug() << "ensureDbSchema failed -> fallback to file";
        }
    }

    loadContacts();   // выберет DB или File
    refreshTable();   // без фильтра
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ====== File ======
void MainWindow::loadContactsFromFile()
{
    m_book.loadFromFile(m_dataFile.toStdString());
    m_contactDbIds.clear(); // в file-режиме id нет
}

void MainWindow::saveContactsToFile()
{
    if (!m_book.saveToFile(m_dataFile.toStdString()))
    {
        QMessageBox::warning(this,
                             tr("Ошибка"),
                             tr("Не удалось сохранить файл контактов:\n%1")
                                 .arg(m_dataFile));
    }
}

// ====== DB helpers ======
static QSqlDatabase dbConn()
{
    return QSqlDatabase::database("phonebook_conn");
}

bool MainWindow::ensureDbSchema()
{
    QSqlDatabase db = dbConn();
    if (!db.isOpen()) return false;

    QSqlQuery q(db);

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
    )")) {
        qDebug() << "Schema contacts error:" << q.lastError().text();
        return false;
    }

    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS phones(
            id SERIAL PRIMARY KEY,
            contact_id INTEGER NOT NULL REFERENCES contacts(id) ON DELETE CASCADE,
            number TEXT NOT NULL,
            type   TEXT NOT NULL
        );
    )")) {
        qDebug() << "Schema phones error:" << q.lastError().text();
        return false;
    }

    return true;
}

bool MainWindow::importFromFileToDbIfEmpty()
{
    QSqlDatabase db = QSqlDatabase::database("phonebook_conn");
    if (!db.isOpen())
        return false;

    // 1) Проверяем: пустая ли таблица contacts
    QSqlQuery q(db);
    if (!q.exec("SELECT COUNT(*) FROM contacts;") || !q.next()) {
        qDebug() << "count contacts failed:" << q.lastError().text();
        return false;
    }

    int count = q.value(0).toInt();
    if (count > 0) {
        qDebug() << "DB not empty -> import skipped";
        return true;
    }

    // 2) Читаем контакты из файла (как в задаче 2)
    ContactBook tmp;
    tmp.loadFromFile(m_dataFile.toStdString());
    const auto &list = tmp.contacts();

    if (list.empty()) {
        qDebug() << "File empty -> nothing to import";
        return true;
    }

    qDebug() << "Importing" << (int)list.size() << "contacts from file to DB...";

    // 3) Импортируем по одному (БЕЗ общей транзакции!)
    for (const auto &c : list) {
        if (!insertContactToDb(c)) {
            qDebug() << "Import failed on email:" << QString::fromStdString(c.email());
            return false;
        }
    }

    qDebug() << "Import done";
    return true;
}

bool MainWindow::loadContactsFromDb()
{
    QSqlDatabase db = dbConn();
    if (!db.isOpen()) return false;

    // очистим текущие данные (проще всего — пересоздать ContactBook)
    m_book = ContactBook{};
    m_contactDbIds.clear();

    QSqlQuery qc(db);
    if (!qc.exec(R"(
        SELECT id, last_name, first_name, middle_name, address,
               COALESCE(to_char(birth_date,'YYYY-MM-DD'), '') AS birth_date,
               email
        FROM contacts
        ORDER BY id;
    )")) {
        qDebug() << "load contacts error:" << qc.lastError().text();
        return false;
    }

    while (qc.next()) {
        int id = qc.value(0).toInt();
        std::string ln  = qc.value(1).toString().toStdString();
        std::string fn  = qc.value(2).toString().toStdString();
        std::string mn  = qc.value(3).toString().toStdString();
        std::string adr = qc.value(4).toString().toStdString();
        std::string bd  = qc.value(5).toString().toStdString();
        std::string em  = qc.value(6).toString().toStdString();

        Date d = Date::fromString(bd);
        if (!d.isValid()) {
            d = Date::fromString("2000-01-01");
        }

        Contact c(ln, fn, mn, adr, d, em);

        QSqlQuery qp(db);
        qp.prepare(R"(
            SELECT number, type
            FROM phones
            WHERE contact_id = :cid
            ORDER BY id;
        )");
        qp.bindValue(":cid", id);

        if (!qp.exec()) {
            qDebug() << "load phones error:" << qp.lastError().text();
            return false;
        }

        while (qp.next()) {
            std::string num = qp.value(0).toString().toStdString();
            std::string typ = qp.value(1).toString().toStdString();
            PhoneType t = PhoneNumber::stringToType(typ);
            c.addPhone(PhoneNumber(num, t));
        }

        m_book.addContact(c);
        m_contactDbIds.push_back(id);
    }

    return true;
}

bool MainWindow::insertContactToDb(const Contact &c)
{
    QSqlDatabase db = dbConn();
    if (!db.isOpen()) return false;

    if (!db.transaction()) {
        qDebug() << "transaction start failed:" << db.lastError().text();
        return false;
    }

    QSqlQuery q(db);
    q.prepare(R"(
        INSERT INTO contacts(last_name, first_name, middle_name, address, birth_date, email)
        VALUES(:ln, :fn, :mn, :adr, :bd, :em)
        RETURNING id;
    )");

    q.bindValue(":ln",  QString::fromStdString(c.lastName()));
    q.bindValue(":fn",  QString::fromStdString(c.firstName()));
    q.bindValue(":mn",  QString::fromStdString(c.middleName()));
    q.bindValue(":adr", QString::fromStdString(c.address()));
    q.bindValue(":bd",  QString::fromStdString(c.birthDate().toString()));
    q.bindValue(":em",  QString::fromStdString(c.email()));

    if (!q.exec() || !q.next()) {
        qDebug() << "insert contact failed:" << q.lastError().text();
        db.rollback();
        return false;
    }

    int newId = q.value(0).toInt();

    const auto &phones = c.phones();
    for (const auto &ph : phones) {
        QSqlQuery qp(db);
        qp.prepare(R"(
            INSERT INTO phones(contact_id, number, type)
            VALUES(:cid, :num, :typ);
        )");
        qp.bindValue(":cid", newId);
        qp.bindValue(":num", QString::fromStdString(ph.number()));
        qp.bindValue(":typ", QString::fromStdString(PhoneNumber::typeToString(ph.type())));

        if (!qp.exec()) {
            qDebug() << "insert phone failed:" << qp.lastError().text();
            db.rollback();
            return false;
        }
    }

    if (!db.commit()) {
        qDebug() << "commit failed:" << db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

bool MainWindow::updateContactInDb(int contactId, const Contact &c)
{
    QSqlDatabase db = dbConn();
    if (!db.isOpen()) return false;

    if (!db.transaction()) return false;

    QSqlQuery q(db);
    q.prepare(R"(
        UPDATE contacts
        SET last_name=:ln, first_name=:fn, middle_name=:mn,
            address=:adr, birth_date=:bd, email=:em
        WHERE id=:id;
    )");
    q.bindValue(":ln",  QString::fromStdString(c.lastName()));
    q.bindValue(":fn",  QString::fromStdString(c.firstName()));
    q.bindValue(":mn",  QString::fromStdString(c.middleName()));
    q.bindValue(":adr", QString::fromStdString(c.address()));
    q.bindValue(":bd",  QString::fromStdString(c.birthDate().toString()));
    q.bindValue(":em",  QString::fromStdString(c.email()));
    q.bindValue(":id",  contactId);

    if (!q.exec()) {
        qDebug() << "update contact failed:" << q.lastError().text();
        db.rollback();
        return false;
    }

    // Переписываем телефоны целиком (проще и надёжнее)
    QSqlQuery qdel(db);
    qdel.prepare("DELETE FROM phones WHERE contact_id=:id;");
    qdel.bindValue(":id", contactId);
    if (!qdel.exec()) {
        qDebug() << "delete phones failed:" << qdel.lastError().text();
        db.rollback();
        return false;
    }

    const auto &phones = c.phones();
    for (const auto &ph : phones) {
        QSqlQuery qp(db);
        qp.prepare(R"(
            INSERT INTO phones(contact_id, number, type)
            VALUES(:cid, :num, :typ);
        )");
        qp.bindValue(":cid", contactId);
        qp.bindValue(":num", QString::fromStdString(ph.number()));
        qp.bindValue(":typ", QString::fromStdString(PhoneNumber::typeToString(ph.type())));
        if (!qp.exec()) {
            qDebug() << "insert phone failed:" << qp.lastError().text();
            db.rollback();
            return false;
        }
    }

    if (!db.commit()) {
        db.rollback();
        return false;
    }

    return true;
}

bool MainWindow::deleteContactFromDb(int contactId)
{
    QSqlDatabase db = dbConn();
    if (!db.isOpen()) return false;

    QSqlQuery q(db);
    q.prepare("DELETE FROM contacts WHERE id=:id;");
    q.bindValue(":id", contactId);
    if (!q.exec()) {
        qDebug() << "delete contact failed:" << q.lastError().text();
        return false;
    }
    return true;
}

// ====== общий выбор режима ======
void MainWindow::loadContacts()
{
    if (m_useDb) {
        if (!loadContactsFromDb()) {
            // если что-то пошло не так — откатимся на файл
            m_useDb = false;
            loadContactsFromFile();
        }
    } else {
        loadContactsFromFile();
    }
}

void MainWindow::saveContacts()
{
    // В DB-режиме файл не трогаем (по требованию задачи 3 “не удаляя хранение в файл”:
    // это значит, что file-режим остаётся и работает, но DB-режим не обязан писать в файл.)
    if (!m_useDb) {
        saveContactsToFile();
    }
}

// Перерисовка таблицы с учётом текущего фильтра
void MainWindow::refreshTable(const QString &filter)
{
    const auto &list = m_book.contacts();

    m_rowToIndex.clear();
    m_lastFilter = filter;

    const QString f = filter.toLower();

    for (std::size_t i = 0; i < list.size(); ++i)
    {
        const Contact &c = list[i];

        QString all = QString::fromStdString(
            c.lastName()  + " " +
            c.firstName() + " " +
            c.middleName() + " " +
            c.address()   + " " +
            c.email()
            );

        const auto &phones = c.phones();
        for (const auto &ph : phones)
            all += " " + QString::fromStdString(ph.number());

        if (!f.isEmpty() && !all.toLower().contains(f))
            continue;

        m_rowToIndex.push_back(i);
    }

    ui->tableContacts->clearContents();
    ui->tableContacts->setRowCount(static_cast<int>(m_rowToIndex.size()));

    for (int row = 0; row < static_cast<int>(m_rowToIndex.size()); ++row)
    {
        std::size_t idx = m_rowToIndex[static_cast<std::size_t>(row)];
        const Contact &c = list[idx];

        auto setCell = [&](int col, const std::string &text, bool storeId = false)
        {
            auto *item = new QTableWidgetItem(QString::fromStdString(text));

            // В DB-режиме на первой колонке храним contact_id в UserRole
            if (storeId && m_useDb && idx < m_contactDbIds.size()) {
                item->setData(Qt::UserRole, m_contactDbIds[idx]);
            }

            ui->tableContacts->setItem(row, col, item);
        };

        setCell(0, c.lastName(), true); // id прячем тут
        setCell(1, c.firstName());
        setCell(2, c.middleName());
        setCell(3, c.address());
        setCell(4, c.birthDate().toString());
        setCell(5, c.email());

        QString phonesStr;
        const auto &phones = c.phones();
        for (std::size_t i = 0; i < phones.size(); ++i)
        {
            if (i != 0)
                phonesStr += "; ";
            phonesStr += QString::fromStdString(phones[i].number());
        }

        auto *phonesItem = new QTableWidgetItem(phonesStr);
        ui->tableContacts->setItem(row, 6, phonesItem);
    }

    ui->tableContacts->resizeColumnsToContents();
}

// ===================== СЛОТЫ КНОПОК =====================

void MainWindow::on_btnAdd_clicked()
{
    ContactDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        Contact c = dlg.contact();

        if (m_useDb) {
            if (!insertContactToDb(c)) {
                QMessageBox::warning(this, tr("DB"),
                                     tr("Не удалось добавить контакт в БД."));
                return;
            }
            loadContacts(); // перечитать из БД, чтобы обновить id
            refreshTable(m_lastFilter);
            return;
        }

        m_book.addContact(c);
        saveContacts();
        refreshTable(m_lastFilter);
    }
}

void MainWindow::on_btnEdit_clicked()
{
    int row = ui->tableContacts->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(this, tr("Редактирование"),
                             tr("Сначала выберите контакт в таблице."));
        return;
    }
    if (row >= static_cast<int>(m_rowToIndex.size()))
        return;

    std::size_t idx = m_rowToIndex[static_cast<std::size_t>(row)];
    const auto &list = m_book.contacts();
    if (idx >= list.size())
        return;

    ContactDialog dlg(this);
    dlg.setContact(list[idx]);

    if (dlg.exec() == QDialog::Accepted)
    {
        Contact c = dlg.contact();

        if (m_useDb) {
            // достаём id из таблицы (UserRole хранится в колонке 0)
            QTableWidgetItem *first = ui->tableContacts->item(row, 0);
            int contactId = first ? first->data(Qt::UserRole).toInt() : -1;
            if (contactId <= 0) {
                QMessageBox::warning(this, tr("DB"), tr("Не найден contact_id."));
                return;
            }

            if (!updateContactInDb(contactId, c)) {
                QMessageBox::warning(this, tr("DB"),
                                     tr("Не удалось обновить контакт в БД."));
                return;
            }

            loadContacts();
            refreshTable(m_lastFilter);
            return;
        }

        m_book.updateContact(idx, c);
        saveContacts();
        refreshTable(m_lastFilter);
    }
}

void MainWindow::on_btnDelete_clicked()
{
    int row = ui->tableContacts->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(this, tr("Удаление"),
                             tr("Сначала выберите контакт в таблице."));
        return;
    }
    if (row >= static_cast<int>(m_rowToIndex.size()))
        return;

    if (QMessageBox::question(this, tr("Удаление"),
                              tr("Удалить выбранный контакт?"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No)
        != QMessageBox::Yes)
    {
        return;
    }

    std::size_t idx = m_rowToIndex[static_cast<std::size_t>(row)];

    if (m_useDb) {
        QTableWidgetItem *first = ui->tableContacts->item(row, 0);
        int contactId = first ? first->data(Qt::UserRole).toInt() : -1;
        if (contactId <= 0) {
            QMessageBox::warning(this, tr("DB"), tr("Не найден contact_id."));
            return;
        }

        if (!deleteContactFromDb(contactId)) {
            QMessageBox::warning(this, tr("DB"),
                                 tr("Не удалось удалить контакт из БД."));
            return;
        }

        loadContacts();
        refreshTable(m_lastFilter);
        return;
    }

    m_book.removeContact(idx);
    saveContacts();
    refreshTable(m_lastFilter);
}

// ===== ПОИСК =====
void MainWindow::on_btnSearch_clicked()
{
    bool ok = false;
    QString text = QInputDialog::getText(
        this,
        tr("Поиск"),
        tr("Введите строку для поиска\n"
           "(ФИО, адрес, e-mail или телефон):"),
        QLineEdit::Normal,
        m_lastFilter,
        &ok
        );

    if (!ok)
        return;

    text = text.trimmed();
    refreshTable(text);
}

// ===== СОРТИРОВКА =====
void MainWindow::on_btnSort_clicked()
{
    // В DB-режиме сортировку ты сейчас делаешь в памяти через ContactBook (как и раньше)
    // Это ок для курсовой. Если захочешь — потом перенесём ORDER BY в SQL.

    QStringList fields;
    fields << tr("Фамилия") << tr("Дата рождения");

    bool ok = false;
    QString fieldStr = QInputDialog::getItem(
        this,
        tr("Сортировка"),
        tr("Поле для сортировки:"),
        fields,
        0,
        false,
        &ok
        );

    if (!ok)
        return;

    QStringList dirs;
    dirs << tr("По возрастанию") << tr("По убыванию");

    QString dirStr = QInputDialog::getItem(
        this,
        tr("Сортировка"),
        tr("Направление:"),
        dirs,
        0,
        false,
        &ok
        );

    if (!ok)
        return;

    bool asc = !dirStr.startsWith(tr("По убыв"));

    SortField field = SortField::LastName;
    if (fieldStr.startsWith(tr("Дата")))
        field = SortField::BirthDate;

    m_book.sortBy(field, asc);
    refreshTable(m_lastFilter);
}

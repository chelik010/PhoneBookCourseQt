#include "ContactBook.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QString>


// Формат файла (текстовый, очень простой):
// CONTACT
// lastName
// firstName
// middleName
// address
// yyyy-mm-dd
// email
// phonesCount
// number|typeString   (повтор phonesCount раз)

bool ContactBook::loadFromFile(const std::string& fileName)
{
    m_contacts.clear();

    QFile file(QString::fromStdString(fileName));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;    // файл не найден → ok, просто пустой справочник

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line != "CONTACT")
            continue;

        QString ln   = in.readLine();
        QString fn   = in.readLine();
        QString mn   = in.readLine();
        QString addr = in.readLine();
        QString bday = in.readLine();
        QString mail = in.readLine();

        if (ln.isNull() || fn.isNull() || mn.isNull()
            || addr.isNull() || bday.isNull() || mail.isNull())
        {
            return false; // файл оборван
        }

        QString phoneCountStr = in.readLine();
        if (phoneCountStr.isNull())
            return false;

        int phonesCount = phoneCountStr.toInt();

        Contact c(
            ln.toStdString(),
            fn.toStdString(),
            mn.toStdString(),
            addr.toStdString(),
            Date::fromString(bday.toStdString()),
            mail.toStdString()
            );

        for (int i = 0; i < phonesCount; ++i)
        {
            QString pLine = in.readLine();
            if (pLine.isNull()) break;

            QStringList parts = pLine.split('|');
            if (parts.size() < 2) continue;

            std::string number = parts[0].trimmed().toStdString();
            std::string typeStr = parts[1].trimmed().toStdString();

            PhoneType pt = PhoneNumber::stringToType(typeStr);
            c.addPhone(PhoneNumber(number, pt));
        }

        m_contacts.push_back(c);
    }

    return true;
}

bool ContactBook::saveToFile(const std::string& fileName) const
{
    QFile file(QString::fromStdString(fileName));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);

    for (const auto& c : m_contacts)
    {
        out << "CONTACT\n";
        out << QString::fromStdString(c.lastName())   << "\n";
        out << QString::fromStdString(c.firstName())  << "\n";
        out << QString::fromStdString(c.middleName()) << "\n";
        out << QString::fromStdString(c.address())    << "\n";
        out << QString::fromStdString(c.birthDate().toString()) << "\n";
        out << QString::fromStdString(c.email())      << "\n";

        const auto &phones = c.phones();
        out << phones.size() << "\n";

        for (const auto &ph : phones)
        {
            out << QString::fromStdString(ph.number())
            << "|" << QString::fromStdString(
                PhoneNumber::typeToString(ph.type())
                ) << "\n";
        }
    }

    return true;
}

void ContactBook::addContact(const Contact& c)
{
    m_contacts.push_back(c);
}

bool ContactBook::removeContact(std::size_t index)
{
    if (index >= m_contacts.size())
        return false;
    m_contacts.erase(m_contacts.begin() + static_cast<long>(index));
    return true;
}

bool ContactBook::updateContact(std::size_t index, const Contact& c)
{
    if (index >= m_contacts.size())
        return false;
    m_contacts[index] = c;
    return true;
}

std::vector<std::size_t> ContactBook::find(const std::string& text) const
{
    std::vector<std::size_t> result;
    if (text.empty())
        return result;

    for (std::size_t i = 0; i < m_contacts.size(); ++i)
    {
        const auto& c = m_contacts[i];
        auto inStr = [&](const std::string& s)
        {
            return s.find(text) != std::string::npos;
        };

        bool match = inStr(c.lastName()) ||
                     inStr(c.firstName()) ||
                     inStr(c.middleName()) ||
                     inStr(c.email()) ||
                     inStr(c.address());

        if (!match)
        {
            for (const auto& ph : c.phones())
            {
                if (ph.number().find(text) != std::string::npos)
                {
                    match = true;
                    break;
                }
            }
        }

        if (match)
            result.push_back(i);
    }

    return result;
}

void ContactBook::sortBy(SortField field, bool ascending)
{
    auto cmp = [&](const Contact& a, const Contact& b)
    {
        bool less = false;
        bool greater = false;

        switch (field)
        {
        case SortField::LastName:
            less = a.lastName() < b.lastName();
            greater = b.lastName() < a.lastName();
            break;

        case SortField::BirthDate:
        {
            const auto& da = a.birthDate();
            const auto& db = b.birthDate();

            if (da.year != db.year) { less = da.year < db.year; greater = db.year < da.year; }
            else if (da.month != db.month) { less = da.month < db.month; greater = db.month < da.month; }
            else { less = da.day < db.day; greater = db.day < da.day; }
            break;
        }
        }

        return ascending ? less : greater;
    };

    std::sort(m_contacts.begin(), m_contacts.end(), cmp);
}


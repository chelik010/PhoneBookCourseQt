#include "ContactBook.h"
#include "Validator.h"
#include <fstream>
#include <algorithm>


static std::string trimLine(std::string s) {
    while (!s.empty() && (s.back()=='\r' || s.back()=='\n')) s.pop_back();
    return s;
}

bool ContactBook::loadFromFile(const std::string& fileName)
{
    m_contacts.clear();

    std::ifstream in(fileName);
    if (!in.is_open())
        return false;

    std::string line;
    while (std::getline(in, line))
    {
        line = trimLine(line);
        if (line != "CONTACT")
            continue;

        std::string ln, fn, mn, addr, bday, mail;
        if (!std::getline(in, ln)) return false;
        if (!std::getline(in, fn)) return false;
        if (!std::getline(in, mn)) return false;
        if (!std::getline(in, addr)) return false;
        if (!std::getline(in, bday)) return false;
        if (!std::getline(in, mail)) return false;

        ln   = trimLine(ln);
        fn   = trimLine(fn);
        mn   = trimLine(mn);
        addr = trimLine(addr);
        bday = trimLine(bday);
        mail = trimLine(mail);

        std::string phoneCountStr;
        if (!std::getline(in, phoneCountStr)) return false;
        phoneCountStr = trimLine(phoneCountStr);

        int phonesCount = 0;
        try { phonesCount = std::stoi(phoneCountStr); }
        catch (...) { return false; }

        Contact c(
            ln, fn, mn, addr,
            Date::fromString(bday),
            mail
            );

        for (int i = 0; i < phonesCount; ++i)
        {
            std::string pLine;
            if (!std::getline(in, pLine)) break;
            pLine = trimLine(pLine);

            auto pos = pLine.find('|');
            if (pos == std::string::npos) continue;

            std::string number = pLine.substr(0, pos);
            std::string typeStr = pLine.substr(pos + 1);

            number = Validator::trim(number);
            typeStr = Validator::trim(typeStr);

            PhoneType pt = PhoneNumber::stringToType(typeStr);
            c.addPhone(PhoneNumber(number, pt));
        }

        m_contacts.push_back(std::move(c));
    }

    return true;
}

bool ContactBook::saveToFile(const std::string& fileName) const
{
    std::ofstream out(fileName, std::ios::trunc);
    if (!out.is_open())
        return false;

    for (const auto& c : m_contacts)
    {
        out << "CONTACT\n";
        out << c.lastName() << "\n";
        out << c.firstName() << "\n";
        out << c.middleName() << "\n";
        out << c.address() << "\n";
        out << c.birthDate().toString() << "\n";
        out << c.email() << "\n";

        const auto& phones = c.phones();
        out << phones.size() << "\n";
        for (const auto& ph : phones)
        {
            out << ph.number() << "|"
                << PhoneNumber::typeToString(ph.type()) << "\n";
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


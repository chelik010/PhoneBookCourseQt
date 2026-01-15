#include "Contact.h"
#include <iostream>

Contact::Contact(std::string lastName,
                 std::string firstName,
                 std::string middleName,
                 std::string address,
                 Date        birthDate,
                 std::string email)
    : m_lastName(std::move(lastName))
    , m_firstName(std::move(firstName))
    , m_middleName(std::move(middleName))
    , m_address(std::move(address))
    , m_birthDate(birthDate)
    , m_email(std::move(email))
{
}

void Contact::addPhone(const PhoneNumber& p)
{
    m_phones.push_back(p);
}

void Contact::clearPhones()
{
    m_phones.clear();
}

void Contact::print(int index) const
{
    std::cout << "----------------------------------------\n";
    std::cout << "#" << index << "\n";
    std::cout << "Фамилия:       " << m_lastName  << "\n";
    std::cout << "Имя:           " << m_firstName << "\n";
    std::cout << "Отчество:      " << m_middleName<< "\n";
    std::cout << "Адрес:         " << m_address   << "\n";
    std::cout << "Дата рождения: " << m_birthDate.toString() << "\n";
    std::cout << "E-mail:        " << m_email     << "\n";
    std::cout << "Телефоны:\n";
    for (std::size_t i = 0; i < m_phones.size(); ++i)
    {
        const auto& ph = m_phones[i];
        std::cout << "  [" << i << "] "
                  << ph.number() << " ("
                  << PhoneNumber::typeToString(ph.type()) << ")\n";
    }
}

#pragma once
#include <string>
#include <vector>
#include "Date.h"
#include "PhoneNumber.h"

class Contact
{
public:
    Contact() = default;

    Contact(std::string lastName,
            std::string firstName,
            std::string middleName,
            std::string address,
            Date        birthDate,
            std::string email);

    const std::string& lastName() const  { return m_lastName; }
    const std::string& firstName() const { return m_firstName; }
    const std::string& middleName() const{ return m_middleName; }
    const std::string& address() const   { return m_address; }
    const Date&        birthDate() const { return m_birthDate; }
    const std::string& email() const     { return m_email; }
    const std::vector<PhoneNumber>& phones() const { return m_phones; }

    void setLastName(const std::string& v)  { m_lastName  = v; }
    void setFirstName(const std::string& v) { m_firstName = v; }
    void setMiddleName(const std::string& v){ m_middleName= v; }
    void setAddress(const std::string& v)   { m_address   = v; }
    void setBirthDate(const Date& d)        { m_birthDate = d; }
    void setEmail(const std::string& v)     { m_email     = v; }

    void addPhone(const PhoneNumber& p);
    void clearPhones();

    void print(int index) const;

private:
    std::string m_lastName;
    std::string m_firstName;
    std::string m_middleName;
    std::string m_address;
    Date        m_birthDate;
    std::string m_email;
    std::vector<PhoneNumber> m_phones;
};

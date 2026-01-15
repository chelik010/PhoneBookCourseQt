#pragma once
#include <string>

enum class PhoneType {
    Mobile,
    Home,
    Work,
    Other
};

class PhoneNumber
{
public:
    PhoneNumber() = default;
    PhoneNumber(std::string number, PhoneType type);

    const std::string& number() const { return m_number; }
    PhoneType type() const { return m_type; }

    void setNumber(const std::string& n) { m_number = n; }
    void setType(PhoneType t) { m_type = t; }

    static std::string typeToString(PhoneType t);
    static PhoneType stringToType(const std::string& s);

private:
    std::string m_number;
    PhoneType   m_type{PhoneType::Mobile};
};

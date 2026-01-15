#include "PhoneNumber.h"

PhoneNumber::PhoneNumber(std::string number, PhoneType type)
    : m_number(std::move(number)), m_type(type)
{
}

std::string PhoneNumber::typeToString(PhoneType t)
{
    switch (t)
    {
    case PhoneType::Mobile: return "mobile";
    case PhoneType::Home:   return "home";
    case PhoneType::Work:   return "work";
    case PhoneType::Other:  return "other";
    }
    return "other";
}

PhoneType PhoneNumber::stringToType(const std::string& s)
{
    if (s == "mobile") return PhoneType::Mobile;
    if (s == "home")   return PhoneType::Home;
    if (s == "work")   return PhoneType::Work;
    return PhoneType::Other;
}

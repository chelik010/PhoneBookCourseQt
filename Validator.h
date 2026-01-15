#pragma once
#include <string>
#include "Date.h"

class Validator
{
public:
    // ФИО
    static bool isValidName(const std::string& raw);

    // Телефон
    static bool isValidPhone(const std::string& raw);

    // E-mail
    static bool isValidEmail(const std::string& raw);

    // Дата рождения (валидная дата + < текущей)
    static bool isValidBirthDate(const Date& d);

    // Вспомогательное
    static std::string trim(const std::string& s);
};

#include "Validator.h"
#include <regex>
#include <cctype>
#include <chrono>

// Преобразование UTF-8 → UTF-32 без codecvt
static std::u32string utf8_to_utf32(const std::string& s)
{
    std::u32string result;
    char32_t codepoint = 0;
    int bytes = 0;

    for (unsigned char c : s)
    {
        if (c <= 0x7F)
        {
            if (bytes != 0)
            {
                bytes = 0;
                codepoint = 0;
            }
            result.push_back(c);
        }
        else if ((c >> 5) == 0x6)
        {
            // Начало последовательности длиной 2
            codepoint = c & 0x1F;
            bytes = 1;
        }
        else if ((c >> 4) == 0xE)
        {
            // Начало последовательности длиной 3
            codepoint = c & 0x0F;
            bytes = 2;
        }
        else if ((c >> 3) == 0x1E)
        {
            // Начало последовательности длиной 4
            codepoint = c & 0x07;
            bytes = 3;
        }
        else if ((c >> 6) == 0x2)
        {
            // Продолжение последовательности UTF-8
            codepoint = (codepoint << 6) | (c & 0x3F);
            if (--bytes == 0)
            {
                result.push_back(codepoint);
                codepoint = 0;
            }
        }
        else
        {
            bytes = 0;
            codepoint = 0;
        }
    }

    return result;
}

// trim пробелы по краям
std::string Validator::trim(const std::string& s)
{
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
        ++start;

    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        --end;

    return s.substr(start, end - start);
}

static bool isLetterOrDigit(char32_t cp)
{
    // Цифры
    if (cp >= U'0' && cp <= U'9')
        return true;

    // Латинские буквы
    if ((cp >= U'a' && cp <= U'z') || (cp >= U'A' && cp <= U'Z'))
        return true;

    // Кириллица
    if ((cp >= U'А' && cp <= U'Я') || (cp >= U'а' && cp <= U'я'))
        return true;

    // Ё / ё
    if (cp == U'Ё' || cp == U'ё')
        return true;

    return false;
}

static bool isLetter(char32_t cp)
{
    // Латиница
    if ((cp >= U'a' && cp <= U'z') || (cp >= U'A' && cp <= U'Z'))
        return true;

    // Кириллица
    if ((cp >= U'А' && cp <= U'Я') || (cp >= U'а' && cp <= U'я'))
        return true;

    // Ё / ё
    if (cp == U'Ё' || cp == U'ё')
        return true;

    return false;
}

bool Validator::isValidName(const std::string& raw)
{
    std::string trimmed = trim(raw);
    if (trimmed.empty())
        return false;

    // распаковываем UTF-8 → массив кодпоинтов
    std::u32string s = utf8_to_utf32(trimmed);

    if (s.empty())
        return false;

    // 1) не должен начинаться с дефиса
    if (s.front() == U'-')
        return false;

    // 2) не должен заканчиваться дефисом
    if (s.back() == U'-')
        return false;

    // 3) первый символ должен быть буквой
    if (!isLetter(s.front()))
        return false;

    // 4) Все символы должны быть: буквы / цифры / дефис / пробел
    for (char32_t cp : s)
    {
        if (isLetterOrDigit(cp))
            continue;

        if (cp == U'-' || cp == U' ')
            continue;

        return false; // запрещённый символ
    }

    return true;
}

bool Validator::isValidPhone(const std::string& raw)
{
    std::string s = trim(raw);

    // +7XXXXXXXXXX
    // 8XXXXXXXXXX
    // +7(XXX)XXXXXXX
    // 8(XXX)XXXXXXX
    // +7(XXX)XXX-XX-XX
    // 8(XXX)XXX-XX-XX

    static const std::regex r1(R"(^\+7\d{10}$)");
    static const std::regex r2(R"(^8\d{10}$)");
    static const std::regex r3(R"(^\+7\(\d{3}\)\d{7}$)");
    static const std::regex r4(R"(^8\(\d{3}\)\d{7}$)");
    static const std::regex r5(R"(^\+7\(\d{3}\)\d{3}-\d{2}-\d{2}$)");
    static const std::regex r6(R"(^8\(\d{3}\)\d{3}-\d{2}-\d{2}$)");

    return std::regex_match(s, r1) ||
           std::regex_match(s, r2) ||
           std::regex_match(s, r3) ||
           std::regex_match(s, r4) ||
           std::regex_match(s, r5) ||
           std::regex_match(s, r6);
}


bool Validator::isValidEmail(const std::string& raw)
{
    // 1. Сначала убираем пробелы по краям всей строки
    std::string s = trim(raw);
    if (s.empty())
        return false;

    // 2. Ищем '@'
    auto atPos = s.find('@');
    if (atPos == std::string::npos)
        return false;

    // 3. Отдельно тримим левую и правую части (все пробелы вокруг '@' удаляем)
    std::string left  = trim(s.substr(0, atPos));       // имя пользователя
    std::string right = trim(s.substr(atPos + 1));      // домен

    if (left.empty() || right.empty())
        return false;

    // 4. Внутри имени и домена пробелов быть не должно
    if (left.find(' ') != std::string::npos)
        return false;
    if (right.find(' ') != std::string::npos)
        return false;

    // 5. Проверяем по регекспам: только латинские буквы и цифры
    static const std::regex userRegex(R"(^[A-Za-z0-9]+$)");
    static const std::regex domainRegex(R"(^[A-Za-z0-9]+(\.[A-Za-z0-9]+)*$)");

    if (!std::regex_match(left, userRegex))
        return false;
    if (!std::regex_match(right, domainRegex))
        return false;

    return true;
}

// более полная проверка даты, чем в Date::isValid
bool Validator::isValidBirthDate(const Date& d)
{
    if (d.year <= 0 || d.month < 1 || d.month > 12 || d.day < 1)
        return false;

    auto daysInMonth = [](int year, int month) -> int {
        switch (month)
        {
        case 1: case 3: case 5: case 7:
        case 8: case 10: case 12: return 31;
        case 4: case 6: case 9: case 11: return 30;
        case 2:
        {
            bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            return leap ? 29 : 28;
        }
        default:
            return 31;
        }
    };

    int maxDay = daysInMonth(d.year, d.month);
    if (d.day > maxDay)
        return false;

    // Дата рождения должна быть < текущей даты
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&t);

    int curYear  = tm->tm_year + 1900;
    int curMonth = tm->tm_mon + 1;
    int curDay   = tm->tm_mday;

    if (d.year > curYear) return false;
    if (d.year == curYear && d.month > curMonth) return false;
    if (d.year == curYear && d.month == curMonth && d.day >= curDay) return false;

    return true;
}

#pragma once
#include <string>
#include <iomanip>
#include <sstream>

struct Date
{
    int year{0};
    int month{0};
    int day{0};

    bool isValid() const
    {
        if (year <= 0 || month < 1 || month > 12 || day < 1 || day > 31)
            return false;
        return true;
    }

    std::string toString() const
    {
        std::ostringstream os;
        os << std::setfill('0')
           << std::setw(4) << year << "-"
           << std::setw(2) << month << "-"
           << std::setw(2) << day;
        return os.str();
    }

    static Date fromString(const std::string& s)
    {
        Date d;
        char dash1 = 0, dash2 = 0;
        std::istringstream is(s);
        is >> d.year >> dash1 >> d.month >> dash2 >> d.day;
        return d;
    }
};

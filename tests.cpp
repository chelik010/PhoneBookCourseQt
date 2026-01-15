#include <iostream>
#include <string>
#include <vector>

#include "Validator.h"
#include "Date.h"
#include "ContactBook.h"
#include "Contact.h"
#include "PhoneNumber.h"

void printResult(const std::string& what, bool got, bool expected)
{
    std::cout << what << " -> " << (got ? "true " : "false")
    << " | expected " << (expected ? "true " : "false")
    << "  [" << ((got == expected) ? "OK" : "FAIL") << "]\n";
}

// --- Тесты имён -----------------------------------------------------

void testNames()
{
    std::cout << "\n=== TEST NAMES ===\n";

    struct Case { std::string s; bool ok; };

    std::vector<Case> cases = {
                               {"Иван",              true},
                               {"Иван Иванов",       true},
                               {"Пётр-Петров",       true},
                               {"  Анна-Мария  ",    true},
                               {"John123",           true},
                               {"-Иван",             false},
                               {"Иван-",             false},
                               {"  -Иван  ",         false},
                               {"123Иван",           false},
                               {"Иван!",             false},
                               {"",                  false},
                               };

    for (const auto& c : cases)
    {
        bool got = Validator::isValidName(c.s);
        std::cout << "[Name] \"" << c.s << "\" ";
        printResult("", got, c.ok);
    }
}

// --- Тесты телефонов -----------------------------------------------

void testPhones()
{
    std::cout << "\n=== TEST PHONES ===\n";

    struct Case { std::string s; bool ok; };

    std::vector<Case> cases = {
                               {"+78121234567",        true},
                               {"88121234567",         true},
                               {"+7(812)1234567",      true},
                               {"8(812)1234567",       true},
                               {"+7(812)123-45-67",    true},
                               {"8(812)123-45-67",     true},
                               {"+79991234567",        true},  // мобильный по требованию
                               {"7121234567",          false},
                               {"+7812",               false},
                               {"+7(812)123-45",       false},
                               {"+7(812)123-45-678",   false},
                               {"text",                false},
                               };

    for (const auto& c : cases)
    {
        bool got = Validator::isValidPhone(c.s);
        std::cout << "[Phone] \"" << c.s << "\" ";
        printResult("", got, c.ok);
    }
}

// --- Тесты e-mail ---------------------------------------------------

void testEmails()
{
    std::cout << "\n=== TEST EMAILS ===\n";

    struct Case { std::string s; bool ok; };

    std::vector<Case> cases = {
                               {"user1@domain1",                 true},
                               {"user123@domain123.sub",         true},
                               {"   user1   @   domain1   ",     true},
                               {"user@",                          false},
                               {"@domain",                        false},
                               {"us er@domain",                   false},
                               {"user@do main",                   false},
                               {"user@domain!",                   false},
                               {"",                               false},
                               };

    for (const auto& c : cases)
    {
        bool got = Validator::isValidEmail(c.s);
        std::cout << "[Email] \"" << c.s << "\" ";
        printResult("", got, c.ok);
    }
}

// --- Тесты дат рождения --------------------------------------------

void testDates()
{
    std::cout << "\n=== TEST DATES ===\n";

    struct Case { Date d; bool ok; std::string label; };

    std::vector<Case> cases = {
                               { Date::fromString("2000-01-01"), true,  "2000-01-01" },
                               { Date::fromString("2024-02-29"), true,  "2024-02-29 (leap)" },
                               { Date::fromString("2023-02-29"), false, "2023-02-29" },
                               { Date::fromString("2020-04-31"), false, "2020-04-31" },
                               { Date::fromString("0000-01-01"), false, "0000-01-01" },
                               { Date::fromString("2025-13-01"), false, "2025-13-01" },
                               { Date::fromString("2025-12-00"), false, "2025-12-00" },
                               { Date::fromString("3000-01-01"), false, "3000-01-01 (future)" },
                               };

    for (const auto& c : cases)
    {
        bool got = Validator::isValidBirthDate(c.d);
        std::cout << "[Date] " << c.label << " ";
        printResult("", got, c.ok);
    }
}

// --- Тест ContactBook (save/load round-trip) -----------------------

void testContactBookRoundTrip()
{
    std::cout << "\n=== TEST CONTACTBOOK SAVE/LOAD ===\n";

    const std::string fileName = "test_contacts.txt";

    // Собираем один контакт
    Date d = Date::fromString("2000-01-01");
    Contact c("Иванов", "Пётр", "Николаевич", "Москва", d, "test@mail.ru");
    c.addPhone(PhoneNumber("+79990001122", PhoneType::Mobile));
    c.addPhone(PhoneNumber("8(812)1234567", PhoneType::Home));

    ContactBook book1;
    book1.addContact(c);

    bool saved = book1.saveToFile(fileName);
    printResult("saveToFile", saved, true);

    ContactBook book2;
    bool loaded = book2.loadFromFile(fileName);
    printResult("loadFromFile", loaded, true);

    const auto &list = book2.contacts();
    bool sizeOk = (list.size() == 1);
    printResult("loaded size == 1", sizeOk, true);

    if (!list.empty())
    {
        const Contact &c2 = list[0];
        bool sameLast  = (c2.lastName()  == c.lastName());
        bool sameFirst = (c2.firstName() == c.firstName());
        bool sameEmail = (c2.email()     == c.email());
        bool phonesOk  = (c2.phones().size() == c.phones().size());

        printResult("lastName equal",  sameLast,  true);
        printResult("firstName equal", sameFirst, true);
        printResult("email equal",     sameEmail, true);
        printResult("phones size eq",  phonesOk,  true);
    }
}

int main()
{
    testNames();
    testPhones();
    testEmails();
    testDates();
    testContactBookRoundTrip();

    std::cout << "\n=== TESTS FINISHED ===\n";
    return 0;
}

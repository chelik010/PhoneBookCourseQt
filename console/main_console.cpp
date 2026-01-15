#include <iostream>
#include <string>
#include "ContactBook.h"
#include "Validator.h"

static void printMenu() {
    std::cout << "\n=== PHONEBOOK (Console) ===\n"
              << "1) Load from file\n"
              << "2) Save to file\n"
              << "3) List contacts\n"
              << "4) Add contact\n"
              << "5) Delete contact\n"
              << "6) Search\n"
              << "7) Sort (last name)\n"
              << "0) Exit\n> ";
}

static std::string inputLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int main() {
    ContactBook book;
    const std::string fileName = "contacts.txt";

    while (true) {
        printMenu();
        std::string cmd;
        std::getline(std::cin, cmd);

        if (cmd == "0") break;

        if (cmd == "1") {
            if (book.loadFromFile(fileName)) std::cout << "Loaded.\n";
            else std::cout << "Load failed (file may not exist).\n";
        }
        else if (cmd == "2") {
            if (book.saveToFile(fileName)) std::cout << "Saved.\n";
            else std::cout << "Save failed.\n";
        }
        else if (cmd == "3") {
            const auto& list = book.contacts();
            for (size_t i = 0; i < list.size(); ++i) list[i].print((int)i);
        }
        else if (cmd == "4") {
            std::string ln = inputLine("Last name*: ");
            std::string fn = inputLine("First name*: ");
            std::string mn = inputLine("Middle name: ");
            std::string adr= inputLine("Address: ");
            std::string bd = inputLine("Birth date (yyyy-mm-dd): ");
            std::string em = inputLine("Email*: ");
            std::string ph = inputLine("Phone*: ");
            std::string pt = inputLine("Phone type (mobile/home/work/other): ");

            if (!Validator::isValidName(ln) || !Validator::isValidName(fn)) {
                std::cout << "ERROR: invalid first/last name\n"; continue;
            }
            if (!mn.empty() && !Validator::isValidName(mn)) {
                std::cout << "ERROR: invalid middle name\n"; continue;
            }
            if (!Validator::isValidEmail(em)) {
                std::cout << "ERROR: invalid email\n"; continue;
            }
            if (!Validator::isValidPhone(ph)) {
                std::cout << "ERROR: invalid phone\n"; continue;
            }
            Date d = Date::fromString(bd);
            if (!d.isValid() || !Validator::isValidBirthDate(d)) {
                std::cout << "ERROR: invalid birth date\n"; continue;
            }

            Contact c(ln, fn, mn, adr, d, em);
            c.addPhone(PhoneNumber(ph, PhoneNumber::stringToType(pt)));

            book.addContact(c);
            std::cout << "Added.\n";
        }
        else if (cmd == "5") {
            std::string s = inputLine("Index to delete: ");
            size_t idx = (size_t)std::stoul(s);
            if (book.removeContact(idx)) std::cout << "Deleted.\n";
            else std::cout << "No such index.\n";
        }
        else if (cmd == "6") {
            std::string q = inputLine("Search text: ");
            auto hits = book.find(q);
            for (auto idx : hits) book.contacts()[idx].print((int)idx);
            std::cout << "Found: " << hits.size() << "\n";
        }
        else if (cmd == "7") {
            book.sortBy(SortField::LastName, true);
            std::cout << "Sorted.\n";
        }
    }
    return 0;
}

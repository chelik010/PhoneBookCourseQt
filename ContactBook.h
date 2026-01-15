#pragma once
#include <vector>
#include <string>
#include "Contact.h"

enum class SortField {
    LastName,
    BirthDate
};

class ContactBook
{
public:
    bool loadFromFile(const std::string& fileName);
    bool saveToFile(const std::string& fileName) const;

    void addContact(const Contact& c);
    bool removeContact(std::size_t index);
    bool updateContact(std::size_t index, const Contact& c);

    const std::vector<Contact>& contacts() const { return m_contacts; }

    std::vector<std::size_t> find(const std::string& text) const;
    void sortBy(SortField field, bool ascending = true);

private:
    std::vector<Contact> m_contacts;
};

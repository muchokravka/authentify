#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>

struct Date {
    int day{};
    int month{};
    int year{};
};

static bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static bool isValidDate(const Date& d) {
    if (d.year < 1 || d.month < 1 || d.month > 12 || d.day < 1) {
        return false;
    }

    static const int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = daysInMonth[d.month];
    if (d.month == 2 && isLeapYear(d.year)) {
        maxDay = 29;
    }
    return d.day <= maxDay;
}

static std::uint64_t fnv1a64(const std::string& input) {
    constexpr std::uint64_t offset = 14695981039346656037ULL;
    constexpr std::uint64_t prime = 1099511628211ULL;

    std::uint64_t hash = offset;
    for (unsigned char c : input) {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= prime;
    }
    return hash;
}

static std::uint64_t splitMix64(std::uint64_t x) {
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27U)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31U);
}

static std::string formatDate(const Date& date) {
    std::ostringstream os;
    os << std::setw(2) << std::setfill('0') << date.day << '-'
       << std::setw(2) << std::setfill('0') << date.month << '-'
       << std::setw(4) << std::setfill('0') << date.year;
    return os.str();
}

static std::string toBase36(std::uint64_t value) {
    static const char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (value == 0) {
        return "0";
    }

    std::string out;
    while (value > 0) {
        out.push_back(alphabet[value % 36U]);
        value /= 36U;
    }

    std::reverse(out.begin(), out.end());
    return out;
}

static std::string makeCode(std::uint64_t seed, int groups = 4, int groupSize = 4) {
    std::string merged;
    merged.reserve(static_cast<std::size_t>(groups * groupSize));

    std::uint64_t state = seed;
    while (static_cast<int>(merged.size()) < groups * groupSize) {
        state = splitMix64(state);
        merged += toBase36(state);
    }
    merged.resize(static_cast<std::size_t>(groups * groupSize));

    std::ostringstream code;
    for (int i = 0; i < groups; ++i) {
        if (i > 0) {
            code << '-';
        }
        code << merged.substr(static_cast<std::size_t>(i * groupSize), static_cast<std::size_t>(groupSize));
    }
    return code.str();
}

static std::string buildSource(const std::string& password, const Date& date, const std::string& role) {
    std::ostringstream os;
    os << role << '|'
       << std::setw(2) << std::setfill('0') << date.day << '|'
       << std::setw(2) << std::setfill('0') << date.month << '|'
       << std::setw(4) << std::setfill('0') << date.year << '|'
       << password;
    return os.str();
}

static void printBanner() {
    std::cout << "\n"
              << "===============================================\n"
              << "        DAILY IDENTITY CODE GENERATOR\n"
              << "===============================================\n"
              << "Generate matching daily verification codes\n"
              << "for two users + one file encryption code.\n"
              << "(Date + shared password, no time used)\n\n";
}

static Date readDateFromUser() {
    Date date;
    while (true) {
        std::cout << "Enter day   (1-31): ";
        if (!(std::cin >> date.day)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter numbers only.\n\n";
            continue;
        }

        std::cout << "Enter month (1-12): ";
        if (!(std::cin >> date.month)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter numbers only.\n\n";
            continue;
        }

        std::cout << "Enter year  (e.g. 2026): ";
        if (!(std::cin >> date.year)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter numbers only.\n\n";
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (isValidDate(date)) {
            break;
        }
        std::cout << "That date is not valid. Please try again.\n\n";
    }
    return date;
}

int main() {
    try {
        printBanner();

        const Date date = readDateFromUser();

        std::string sharedPassword;

        std::cout << "\nEnter shared secret password: ";
        std::getline(std::cin, sharedPassword);

        if (sharedPassword.empty()) {
            std::cerr << "\nError: Password cannot be empty.\n";
            return 1;
        }

        const std::uint64_t user1Seed = fnv1a64(buildSource(sharedPassword, date, "USER1"));
        const std::uint64_t user2Seed = fnv1a64(buildSource(sharedPassword, date, "USER2"));
        const std::uint64_t fileSeed = fnv1a64(buildSource(sharedPassword, date, "FILE_ENCRYPT"));

        const std::string user1Code = makeCode(user1Seed);
        const std::string user2Code = makeCode(user2Seed);
        const std::string fileCode = makeCode(fileSeed);

        std::cout << "\n--------------- GENERATED CODES ---------------\n";
        std::cout << "Date: " << formatDate(date) << "\n\n";
        std::cout << "User #1 Verification Code : " << user1Code << "\n";
        std::cout << "User #2 Verification Code : " << user2Code << "\n";
        std::cout << "File Encryption Code      : " << fileCode << "\n";
        std::cout << "-----------------------------------------------\n";

        std::cout << "\nCopy-all line (easy clipboard use):\n";
        std::cout << formatDate(date) << '|' << user1Code << '|' << user2Code << '|' << fileCode << "\n";

        std::cout << "\nDone. Use the same date + password on another machine to regenerate identical codes.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

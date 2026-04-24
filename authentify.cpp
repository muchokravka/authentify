#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <algorithm>
#include <array>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Date {
    int day{};
    int month{};
    int year{};
};

static std::uint64_t splitMix64(std::uint64_t x);

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

static std::uint32_t rotr32(std::uint32_t value, std::uint32_t shift) {
    return (value >> shift) | (value << (32U - shift));
}

static std::array<std::uint8_t, 32> sha256(const std::vector<std::uint8_t>& data) {
    static constexpr std::array<std::uint32_t, 64> k = {
        0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU, 0x59f111f1U, 0x923f82a4U,
        0xab1c5ed5U, 0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU,
        0x9bdc06a7U, 0xc19bf174U, 0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU, 0x2de92c6fU,
        0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, 0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
        0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U, 0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU,
        0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U, 0xa2bfe8a1U, 0xa81a664bU,
        0xc24b8b70U, 0xc76c51a3U, 0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U, 0x19a4c116U,
        0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
        0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U, 0x90befffaU, 0xa4506cebU, 0xbef9a3f7U,
        0xc67178f2U};

    std::array<std::uint32_t, 8> h = {
        0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
        0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U};

    std::vector<std::uint8_t> padded = data;
    padded.push_back(0x80U);
    while ((padded.size() % 64U) != 56U) {
        padded.push_back(0x00U);
    }

    const std::uint64_t bitLen = static_cast<std::uint64_t>(data.size()) * 8ULL;
    for (int i = 7; i >= 0; --i) {
        padded.push_back(static_cast<std::uint8_t>((bitLen >> (static_cast<std::uint64_t>(i) * 8ULL)) & 0xFFULL));
    }

    for (std::size_t chunk = 0; chunk < padded.size(); chunk += 64U) {
        std::array<std::uint32_t, 64> w{};

        for (std::size_t i = 0; i < 16U; ++i) {
            const std::size_t idx = chunk + (i * 4U);
            w[i] = (static_cast<std::uint32_t>(padded[idx]) << 24U) |
                   (static_cast<std::uint32_t>(padded[idx + 1U]) << 16U) |
                   (static_cast<std::uint32_t>(padded[idx + 2U]) << 8U) |
                   static_cast<std::uint32_t>(padded[idx + 3U]);
        }

        for (std::size_t i = 16U; i < 64U; ++i) {
            const std::uint32_t s0 = rotr32(w[i - 15U], 7U) ^ rotr32(w[i - 15U], 18U) ^ (w[i - 15U] >> 3U);
            const std::uint32_t s1 = rotr32(w[i - 2U], 17U) ^ rotr32(w[i - 2U], 19U) ^ (w[i - 2U] >> 10U);
            w[i] = w[i - 16U] + s0 + w[i - 7U] + s1;
        }

        std::uint32_t a = h[0];
        std::uint32_t b = h[1];
        std::uint32_t c = h[2];
        std::uint32_t d = h[3];
        std::uint32_t e = h[4];
        std::uint32_t f = h[5];
        std::uint32_t g = h[6];
        std::uint32_t hh = h[7];

        for (std::size_t i = 0; i < 64U; ++i) {
            const std::uint32_t s1 = rotr32(e, 6U) ^ rotr32(e, 11U) ^ rotr32(e, 25U);
            const std::uint32_t ch = (e & f) ^ ((~e) & g);
            const std::uint32_t temp1 = hh + s1 + ch + k[i] + w[i];
            const std::uint32_t s0 = rotr32(a, 2U) ^ rotr32(a, 13U) ^ rotr32(a, 22U);
            const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const std::uint32_t temp2 = s0 + maj;

            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += hh;
    }

    std::array<std::uint8_t, 32> digest{};
    for (std::size_t i = 0; i < h.size(); ++i) {
        digest[i * 4U] = static_cast<std::uint8_t>((h[i] >> 24U) & 0xFFU);
        digest[i * 4U + 1U] = static_cast<std::uint8_t>((h[i] >> 16U) & 0xFFU);
        digest[i * 4U + 2U] = static_cast<std::uint8_t>((h[i] >> 8U) & 0xFFU);
        digest[i * 4U + 3U] = static_cast<std::uint8_t>(h[i] & 0xFFU);
    }
    return digest;
}

static std::vector<std::uint8_t> toBytes(const std::string& text) {
    return std::vector<std::uint8_t>(text.begin(), text.end());
}

static std::array<std::uint8_t, 32> hmacSha256(const std::vector<std::uint8_t>& key,
                                               const std::vector<std::uint8_t>& message) {
    constexpr std::size_t blockSize = 64U;
    std::vector<std::uint8_t> normalizedKey = key;

    if (normalizedKey.size() > blockSize) {
        const auto hashed = sha256(normalizedKey);
        normalizedKey.assign(hashed.begin(), hashed.end());
    }
    normalizedKey.resize(blockSize, 0x00U);

    std::vector<std::uint8_t> oKeyPad(blockSize, 0x5cU);
    std::vector<std::uint8_t> iKeyPad(blockSize, 0x36U);
    for (std::size_t i = 0; i < blockSize; ++i) {
        oKeyPad[i] ^= normalizedKey[i];
        iKeyPad[i] ^= normalizedKey[i];
    }

    std::vector<std::uint8_t> inner(iKeyPad);
    inner.insert(inner.end(), message.begin(), message.end());
    const auto innerHash = sha256(inner);

    std::vector<std::uint8_t> outer(oKeyPad);
    outer.insert(outer.end(), innerHash.begin(), innerHash.end());
    return sha256(outer);
}

static std::array<std::uint8_t, 32> pbkdf2HmacSha256(const std::string& password,
                                                     const std::string& salt,
                                                     std::uint32_t iterations) {
    std::vector<std::uint8_t> saltBytes = toBytes(salt);
    saltBytes.push_back(0x00U);
    saltBytes.push_back(0x00U);
    saltBytes.push_back(0x00U);
    saltBytes.push_back(0x01U);

    const std::vector<std::uint8_t> passBytes = toBytes(password);
    std::array<std::uint8_t, 32> u = hmacSha256(passBytes, saltBytes);
    std::array<std::uint8_t, 32> output = u;

    for (std::uint32_t i = 1U; i < iterations; ++i) {
        std::vector<std::uint8_t> uVec(u.begin(), u.end());
        u = hmacSha256(passBytes, uVec);
        for (std::size_t j = 0; j < output.size(); ++j) {
            output[j] ^= u[j];
        }
    }

    return output;
}

static std::uint64_t deriveSeed(const std::string& password, const Date& date, const std::string& role) {
    constexpr std::uint32_t iterations = 120000U;
    std::ostringstream salt;
    salt << "DailyIdentityCodeGenerator:v2|"
         << role << '|'
         << std::setw(2) << std::setfill('0') << date.day << '|'
         << std::setw(2) << std::setfill('0') << date.month << '|'
         << std::setw(4) << std::setfill('0') << date.year;

    const auto derived = pbkdf2HmacSha256(password, salt.str(), iterations);

    std::uint64_t seedA = 0ULL;
    std::uint64_t seedB = 0ULL;
    for (int i = 0; i < 8; ++i) {
        seedA = (seedA << 8ULL) | static_cast<std::uint64_t>(derived[static_cast<std::size_t>(i)]);
    }
    for (int i = 8; i < 16; ++i) {
        seedB = (seedB << 8ULL) | static_cast<std::uint64_t>(derived[static_cast<std::size_t>(i)]);
    }
    return splitMix64(seedA ^ seedB ^ 0xA5A5A5A5A5A5A5A5ULL);
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

static void printBanner() {
    std::cout << "\n"
              << "===============================================\n"
              << "        DAILY IDENTITY CODE GENERATOR\n"
              << "===============================================\n"
              << "Generate matching daily verification codes\n"
              << "for two users + one file encryption code.\n"
              << "(Date + shared password, no time used)\n"
              << "(Hardened derivation: PBKDF2-HMAC-SHA256)\n\n";
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

        const std::uint64_t user1Seed = deriveSeed(sharedPassword, date, "USER1");
        const std::uint64_t user2Seed = deriveSeed(sharedPassword, date, "USER2");
        const std::uint64_t fileSeed = deriveSeed(sharedPassword, date, "FILE_ENCRYPT");

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

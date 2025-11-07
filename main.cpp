// Lab 10 - String-Based Calculator
// Author: Gonzalo Arroyo
// Description:
// This program reads lines from a file. Each line has two "numbers" written as strings.
// We first check if each string is a valid "double-like" number (with optional sign, digits,
// and optional decimal point with digits on both sides). If both are valid, we add them
// using string arithmetic (no std::stod, no casting to double). Otherwise, we report invalid.
// This matches the lab requirement of staying in string world only.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

struct ParsedNumber {
    bool isNegative;
    std::string intPart;   // digits before decimal
    std::string fracPart;  // digits after decimal (maybe empty)
};

// helper: check if a char is digit
bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// 1) validate according to lab rules
// Valid formats:
//  [+|-]digits
//  [+|-]digits.digits
// decimal point is optional, but if it appears, there must be at least ONE digit on BOTH sides
bool isValidDoubleString(const std::string &s) {
    if (s.empty()) return false;

    size_t i = 0;

    // optional sign
    if (s[i] == '+' || s[i] == '-') {
        i++;
        if (i == s.size()) return false; // sign alone is not valid
    }

    bool sawDigitsBeforeDot = false;
    bool sawDot = false;
    bool sawDigitsAfterDot = false;

    for (; i < s.size(); ++i) {
        char c = s[i];
        if (isDigit(c)) {
            if (!sawDot)
                sawDigitsBeforeDot = true;
            else
                sawDigitsAfterDot = true;
        } else if (c == '.') {
            if (sawDot) {
                // two dots -> invalid
                return false;
            }
            sawDot = true;
        } else {
            // any other char -> invalid
            return false;
        }
    }

    // if there was a dot, must have digits on both sides
    if (sawDot) {
        if (!sawDigitsBeforeDot) return false;
        if (!sawDigitsAfterDot) return false;
    } else {
        // no dot -> must have at least digits
        if (!sawDigitsBeforeDot) return false;
    }

    return true;
}

// remove leading zeros from integer part, but leave at least one digit
std::string trimLeadingZeros(const std::string &s) {
    size_t i = 0;
    while (i + 1 < s.size() && s[i] == '0') {
        i++;
    }
    return s.substr(i);
}

// remove trailing zeros from fraction part
std::string trimTrailingZeros(const std::string &s) {
    if (s.empty()) return s;
    int i = static_cast<int>(s.size()) - 1;
    while (i >= 0 && s[i] == '0') {
        i--;
    }
    if (i < 0) return ""; // all zeros
    return s.substr(0, i + 1);
}

// 2) turn a valid double-string into ParsedNumber
ParsedNumber parseNumber(const std::string &s) {
    ParsedNumber p;
    p.isNegative = false;
    std::string work = s;

    // sign
    if (!work.empty() && (work[0] == '+' || work[0] == '-')) {
        p.isNegative = (work[0] == '-');
        work = work.substr(1);
    }

    // split on dot
    size_t dotPos = work.find('.');
    if (dotPos == std::string::npos) {
        p.intPart = work;
        p.fracPart = "";
    } else {
        p.intPart = work.substr(0, dotPos);
        p.fracPart = work.substr(dotPos + 1);
    }

    // normalize integer part
    p.intPart = trimLeadingZeros(p.intPart);
    // do NOT trim frac here; for addition we might need to pad
    return p;
}
// compare magnitudes of two parsed numbers (ignoring sign)
// return 1 if a > b, 0 if equal, -1 if a < b
int compareMagnitude(const ParsedNumber &a, const ParsedNumber &b) {
    // to compare decimals, we need same frac length
    size_t maxFrac = std::max(a.fracPart.size(), b.fracPart.size());

    // compare integer part by length
    if (a.intPart.size() > b.intPart.size()) return 1;
    if (a.intPart.size() < b.intPart.size()) return -1;

    // same length -> lexicographic
    if (a.intPart > b.intPart) return 1;
    if (a.intPart < b.intPart) return -1;

    // integers equal -> compare fractional
    std::string af = a.fracPart;
    std::string bf = b.fracPart;
    af.append(maxFrac - af.size(), '0');
    bf.append(maxFrac - bf.size(), '0');

    if (af > bf) return 1;
    if (af < bf) return -1;
    return 0;
}

// add magnitudes (a + b), both positive, return normalized parts
ParsedNumber addMagnitudes(const ParsedNumber &a, const ParsedNumber &b) {
    ParsedNumber res;
    res.isNegative = false;

    // 1) pad frac to same length
    size_t maxFrac = std::max(a.fracPart.size(), b.fracPart.size());
    std::string af = a.fracPart;
    std::string bf = b.fracPart;
    af.append(maxFrac - af.size(), '0');
    bf.append(maxFrac - bf.size(), '0');

    // 2) add fractional from right to left
    std::string fracResult(maxFrac, '0');
    int carry = 0;
    for (int i = static_cast<int>(maxFrac) - 1; i >= 0; --i) {
        int d1 = af[i] - '0';
        int d2 = bf[i] - '0';
        int sum = d1 + d2 + carry;
        fracResult[i] = static_cast<char>('0' + (sum % 10));
        carry = sum / 10;
    }

    // 3) add integer parts
    std::string ai = a.intPart;
    std::string bi = b.intPart;
    // pad left with zeros
    size_t maxInt = std::max(ai.size(), bi.size());
    ai.insert(ai.begin(), maxInt - ai.size(), '0');
    bi.insert(bi.begin(), maxInt - bi.size(), '0');

    std::string intResult(maxInt, '0');
    for (int i = static_cast<int>(maxInt) - 1; i >= 0; --i) {
        int d1 = ai[i] - '0';
        int d2 = bi[i] - '0';
        int sum = d1 + d2 + carry;
        intResult[i] = static_cast<char>('0' + (sum % 10));
        carry = sum / 10;
    }
    if (carry > 0) {
        intResult.insert(intResult.begin(), static_cast<char>('0' + carry));
    }

    // normalize
    res.intPart = trimLeadingZeros(intResult);
    res.fracPart = fracResult; // we'll trim trailing zeros at printing time
    return res;
}

// subtract magnitudes: assume a >= b (by magnitude), return a - b
ParsedNumber subtractMagnitudes(const ParsedNumber &a, const ParsedNumber &b) {
    ParsedNumber res;
    res.isNegative = false;

    // pad frac
    size_t maxFrac = std::max(a.fracPart.size(), b.fracPart.size());
    std::string af = a.fracPart;
    std::string bf = b.fracPart;
    af.append(maxFrac - af.size(), '0');
    bf.append(maxFrac - bf.size(), '0');

    // subtract fractional part
    std::string fracResult(maxFrac, '0');
    int borrow = 0;
    for (int i = static_cast<int>(maxFrac) - 1; i >= 0; --i) {
        int d1 = (af[i] - '0') - borrow;
        int d2 = (bf[i] - '0');
        if (d1 < d2) {
            d1 += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        int diff = d1 - d2;
        fracResult[i] = static_cast<char>('0' + diff);
    }

    // subtract integer parts
    std::string ai = a.intPart;
    std::string bi = b.intPart;
    size_t maxInt = std::max(ai.size(), bi.size());
    ai.insert(ai.begin(), maxInt - ai.size(), '0');
    bi.insert(bi.begin(), maxInt - bi.size(), '0');

    std::string intResult(maxInt, '0');
    for (int i = static_cast<int>(maxInt) - 1; i >= 0; --i) {
        int d1 = (ai[i] - '0') - borrow;
        int d2 = (bi[i] - '0');
        if (d1 < d2) {
            d1 += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        int diff = d1 - d2;
        intResult[i] = static_cast<char>('0' + diff);
    }

    // normalize int
    res.intPart = trimLeadingZeros(intResult);
    res.fracPart = fracResult;
    return res;
}

// main add that respects signs
std::string addStringDoubles(const std::string &s1, const std::string &s2) {
    ParsedNumber a = parseNumber(s1);
    ParsedNumber b = parseNumber(s2);

    ParsedNumber result;
    // same sign -> add magnitudes
    if (a.isNegative == b.isNegative) {
        result = addMagnitudes(a, b);
        result.isNegative = a.isNegative; // keep the common sign
    } else {
        // different signs -> actually subtraction
        int cmp = compareMagnitude(a, b);
        if (cmp == 0) {
            // they cancel out -> zero
            return "0";
        } else if (cmp > 0) {
            // |a| > |b| -> result has sign of a
            result = subtractMagnitudes(a, b);
            result.isNegative = a.isNegative;
        } else {
            // |b| > |a| -> result has sign of b
            result = subtractMagnitudes(b, a);
            result.isNegative = b.isNegative;
        }
    }

    // build output string
    std::string out;
    if (result.isNegative && !(result.intPart == "0" && trimTrailingZeros(result.fracPart).empty())) {
        out.push_back('-');
    }
    out += result.intPart;

    std::string trimmedFrac = trimTrailingZeros(result.fracPart);
    if (!trimmedFrac.empty()) {
        out.push_back('.');
        out += trimmedFrac;
    }

    return out;
}

int main() {
    std::string filename;
    std::cout << "Enter input filename: ";
    std::getline(std::cin, filename);

    std::ifstream fin(filename);
    if (!fin.is_open()) {
        std::cerr << "Error: could not open file: " << filename << std::endl;
        return 1;
    }

    std::string line;
    int lineNo = 0;
    while (std::getline(fin, line)) {
        lineNo++;
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string a, b;
        iss >> a >> b;

        if (a.empty() || b.empty()) {
            std::cout << "Line " << lineNo << ": not enough values.\n";
            continue;
        }

        bool validA = isValidDoubleString(a);
        bool validB = isValidDoubleString(b);

        if (!validA || !validB) {
            std::cout << "Line " << lineNo << ": invalid number(s): ";
            if (!validA) std::cout << "'" << a << "' ";
            if (!validB) std::cout << "'" << b << "' ";
            std::cout << "\n";
            continue;
        }

        std::string sum = addStringDoubles(a, b);
        std::cout << "Line " << lineNo << ": " << a << " + " << b << " = " << sum << "\n";
    }

    fin.close();
    return 0;
}
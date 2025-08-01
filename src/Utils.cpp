#include <string>
#include <cctype>

std::string trim(const std::string &str) {
    size_t start = 0;
    while (start < str.size() and std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }

    size_t end = str.size();
    while(end > start and std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        end--;
    }

    return str.substr(start, end - start);
}

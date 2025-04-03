#include "../../include/Core/TopicExtractor.hpp"
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <map>

const std::unordered_set<std::string> TopicExtractor::stopwords = {
    "the", "is", "in", "and", "or", "a", "an", "to", "for", "with",
    "on", "at", "by", "of", "that", "this", "it", "as", "are", "was", "be"
};

namespace {
    std::string clean(const std::string& word) {
        std::string result;
        for (char c : word) {
            if (std::isalnum(c)) result += std::tolower(c);
        }
        return result;
    }

    const std::map<std::string, std::string> aliasMap = {
        {"bye", "goodbye"},
        {"hi", "hello"},
        {"hey", "hello"},
        {"thanks", "thank"},
        {"okay", "ok"},
        {"yeah", "yes"},
        {"nope", "no"},
        {"yep", "yes"}
    };

    std::string normalize(const std::string& word) {
        auto cleaned = clean(word);
        auto it = aliasMap.find(cleaned);
        return it != aliasMap.end() ? it->second : cleaned;
    }
}

std::vector<std::string> TopicExtractor::extract(const std::string& input) {
    std::istringstream iss(input);
    std::string word;
    std::vector<std::string> keywords;

    while (iss >> word) {
        auto norm = normalize(word);
        if (!norm.empty() && !stopwords.count(norm)) {
            keywords.push_back(norm);
        }
    }

    return keywords;
}

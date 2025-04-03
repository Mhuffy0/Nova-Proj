#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>

class TopicExtractor {
public:
    static std::vector<std::string> extract(const std::string& input);
    static void addCustomMapping(const std::string& phrase, const std::string& topic);

private:
    static const std::unordered_set<std::string> stopwords;
    static std::map<std::string, std::string> phraseToTopicMap;
    static std::string clean(const std::string& word);
};

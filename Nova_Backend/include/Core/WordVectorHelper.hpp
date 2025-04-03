#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>
#include <unordered_set>

class WordVectorHelper {
public:
    static std::vector<std::string> tokenize(const std::string& input);
    static std::vector<float> averageVectorFromInput(sqlite3* db, const std::string& input);
    static void storeVector(sqlite3* db, const std::string& word, const std::vector<float>& vec);
    static std::vector<float> fetchVector(sqlite3* db, const std::string& word);
    static float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);


private :
    static const std::unordered_set<std::string> stopwords;
};

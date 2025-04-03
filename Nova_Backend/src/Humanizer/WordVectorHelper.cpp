#include "../../include/Core/WordVectorHelper.hpp"
#include <sstream>
#include <cmath>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <unordered_set>

const std::unordered_set<std::string> WordVectorHelper::stopwords = {
    "the", "is", "in", "and", "or", "a", "an", "to", "for", "with",
    "on", "at", "by", "of", "that", "this", "it", "as", "are", "was", "be"
};

std::vector<float> WordVectorHelper::averageVectorFromInput(sqlite3* db, const std::string& input) {
    std::istringstream iss(input);
    std::string word;
    std::vector<std::vector<float>> vectors;

    while (iss >> word) {
        auto vec = fetchVector(db, word);
        if (!vec.empty()) vectors.push_back(vec);
    }

    if (vectors.empty()) return {};

    size_t dim = vectors[0].size();
    std::vector<float> avg(dim, 0.0f);
    for (const auto& vec : vectors) {
        for (size_t i = 0; i < dim; ++i) {
            avg[i] += vec[i];
        }
    }
    for (float& val : avg) val /= vectors.size();
    return avg;
}

void WordVectorHelper::storeVector(sqlite3* db, const std::string& word, const std::vector<float>& vec) {
    std::string sql = "INSERT OR REPLACE INTO word_vectors (word, vector) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        std::string vecStr;
        for (float f : vec) vecStr += std::to_string(f) + ",";
        vecStr.pop_back();
        sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, vecStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

std::vector<float> WordVectorHelper::fetchVector(sqlite3* db, const std::string& word) {
    std::string sql = "SELECT vector FROM word_vectors WHERE word = ?;";
    sqlite3_stmt* stmt;
    std::vector<float> result;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string vecStr(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            std::istringstream ss(vecStr);
            std::string token;
            while (std::getline(ss, token, ',')) {
                result.push_back(std::stof(token));
            }
        }
        sqlite3_finalize(stmt);
    }

    // fallback: return mock vector with seeded values
    if (result.empty()) {
        std::vector<float> fallback(8, 0.0f);
        for (char c : word) fallback[c % fallback.size()] += 0.1f;
        result = fallback;
    }
    return result;
}

float WordVectorHelper::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size()) return 0.0f;
    float dot = 0.0f, normA = 0.0f, normB = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    if (normA == 0.0f || normB == 0.0f) return 0.0f;
    return dot / (std::sqrt(normA) * std::sqrt(normB));
}

std::vector<std::string> WordVectorHelper::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::string word;
    
    // Convert input to lowercase for uniformity
    std::string processed_input = input;
    std::transform(processed_input.begin(), processed_input.end(), processed_input.begin(), ::tolower);
    
    std::istringstream stream(processed_input);
    while (stream >> word) {
        // Remove punctuation from the word
        word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
        tokens.push_back(word);
    }

    return tokens;
}
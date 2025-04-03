// ResponseSelector.cpp
#include "../../include/Humanizer/ResponseSelector.hpp"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <random>

ResponseSelector::ResponseSelector(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Failed to open DB: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        initializeDB();
    }
}

void ResponseSelector::initializeDB() {
    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS chatbot ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "topic TEXT NOT NULL,"
        "response TEXT NOT NULL,"
        "confidence REAL DEFAULT 0.5,"
        "tag TEXT DEFAULT '',"
        "use_count INTEGER DEFAULT 0,"
        "last_used INTEGER DEFAULT 0"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "DB table creation failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

std::string ResponseSelector::chooseBest(const std::string& input) {
    if (!db) return "";

    std::vector<float> inputVec = nn.vectorize(input);

    std::string sql = "SELECT id, topic, response, confidence, tag, use_count FROM chatbot";
    sqlite3_stmt* stmt;
    std::vector<std::tuple<int, std::string, float>> candidates;
    float bestScore = -1e9f;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            std::string topic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string response = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            float confidence = static_cast<float>(sqlite3_column_double(stmt, 3));
            std::string tag = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

            std::vector<float> topicVec = nn.vectorize(topic);
            float similarity = 0.0f;
            for (int i = 0; i < topicVec.size(); ++i)
                similarity += inputVec[i] * topicVec[i];

            float score = similarity * confidence;

            if (score > bestScore) {
                bestScore = score;
                candidates.clear();
                candidates.emplace_back(id, response, score);
            } else if (score == bestScore) {
                candidates.emplace_back(id, response, score);
            }
        }
        sqlite3_finalize(stmt);
    }

    if (candidates.empty()) return "";

    // Random shuffle among best-scored responses
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, candidates.size() - 1);
    auto [chosenId, chosenResponse, _] = candidates[dist(rng)];

    // Update usage stats
    std::string updateSQL = "UPDATE chatbot SET use_count = use_count + 1, last_used = ? WHERE id = ?";
    sqlite3_stmt* updateStmt;
    if (sqlite3_prepare_v2(db, updateSQL.c_str(), -1, &updateStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(updateStmt, 1, static_cast<int>(std::time(nullptr)));
        sqlite3_bind_int(updateStmt, 2, chosenId);
        sqlite3_step(updateStmt);
        sqlite3_finalize(updateStmt);
    }

    return chosenResponse;
}
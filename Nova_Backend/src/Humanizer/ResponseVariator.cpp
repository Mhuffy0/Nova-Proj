#include "../../include/Humanizer/ResponseVariator.hpp"
#include "../../include/Core/NeuralNet.hpp"
#include "../../include/utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <sqlite3.h>
#include <random>

ResponseVariator::ResponseVariator() {
    rng.seed(std::random_device{}());
    createTablesIfNotExist();
}

void ResponseVariator::createTablesIfNotExist() {
    if (sqlite3_open("D:/Nova_Project/Nova_Backend/chatbot.db", &db) != SQLITE_OK) {
        std::cerr << "DB error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* responseTable = R"(
        CREATE TABLE IF NOT EXISTS responses (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            topic TEXT,
            response TEXT,
            confidence REAL,
            use_count INTEGER DEFAULT 1,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        );
    )";

    const char* vectorTable = R"(
        CREATE TABLE IF NOT EXISTS word_vectors (
            token TEXT PRIMARY KEY,
            v1 REAL, v2 REAL, v3 REAL
        );
    )";

    char* err;
    if (sqlite3_exec(db, responseTable, 0, 0, &err) != SQLITE_OK ||
        sqlite3_exec(db, vectorTable, 0, 0, &err) != SQLITE_OK) {
        std::cerr << "SQL error: " << err << std::endl;
        sqlite3_free(err);
    }
}

#include <random>
#include <iostream>

//super fn
std::string ResponseVariator::getResponse(const std::string& input) {
    std::cout << "Getting response for input: " << input << std::endl;

    // Query the database for responses based on the input (topic)
    const char* sql = "SELECT response, confidence FROM responses WHERE topic = ?;";
    sqlite3_stmt* stmt;
    std::vector<std::pair<std::string, float>> responses;

    // First, check for exact matches
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, input.c_str(), -1, SQLITE_STATIC);

        // Loop through all responses
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string response = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            float confidence = static_cast<float>(sqlite3_column_double(stmt, 1));
            responses.push_back({response, confidence});
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Database query failed: " << sqlite3_errmsg(db) << std::endl;
    }

    // If responses found in the database, select the response with the highest confidence
    if (!responses.empty()) {
        // Sort the responses by confidence (highest first)
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(responses.begin(), responses.end(), g);  // Randomize to avoid bias
        std::sort(responses.begin(), responses.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;  // Sort by confidence
        });

        // Return the response with the highest confidence
        return responses.front().first;
    }

    // No match found in DB, check for a similar word using Levenshtein Distance
    std::cout << "No exact match found in DB. Checking for similar words..." << std::endl;

    std::string closestMatch = findSimilarWord(input);
    if (!closestMatch.empty()) {
        // Return the response corresponding to the closest match
        std::cout << "Found similar word: " << closestMatch << std::endl;
        
        // Query for the response associated with the similar word
        const char* sqlSimilar = "SELECT response FROM responses WHERE topic = ?;";
        if (sqlite3_prepare_v2(db, sqlSimilar, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, closestMatch.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string response = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                sqlite3_finalize(stmt);
                return response; // Return the response corresponding to the similar word
            }
        }
    }

    std::cout << "No similar word found. Generating response using NN..." << std::endl;

    // Use the generateResponseFromNN method
    std::string generatedResponse = generateResponseFromNN(input);

    // If NN fails to generate a meaningful response (empty), fallback to default message
    if (generatedResponse.empty()) {
        return "I don't know yet.";
    }

    // Return the generated response
    return generatedResponse;
}

//string similarity
int ResponseVariator::levenshteinDistance(const std::string& a, const std::string& b) {
    std::vector<std::vector<int>> dist(a.size() + 1, std::vector<int>(b.size() + 1));

    for (int i = 0; i <= a.size(); ++i) {
        for (int j = 0; j <= b.size(); ++j) {
            if (i == 0) {
                dist[i][j] = j;
            } else if (j == 0) {
                dist[i][j] = i;
            } else {
                dist[i][j] = std::min({
                    dist[i - 1][j] + 1, // Deletion
                    dist[i][j - 1] + 1, // Insertion
                    dist[i - 1][j - 1] + (a[i - 1] != b[j - 1]) // Substitution
                });
            }
        }
    }

    return dist[a.size()][b.size()];
}

//find
std::string ResponseVariator::findSimilarWord(const std::string& input) {
    std::string closestWord;
    int minDistance = INT_MAX;

    // Query database for words (assuming you are looking for the closest topic)
    const char* sql = "SELECT topic FROM responses;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string word = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

            // Calculate Levenshtein distance between the input and the word from DB
            int distance = levenshteinDistance(input, word);
            if (distance < minDistance) {
                minDistance = distance;
                closestWord = word;
            }
        }
        sqlite3_finalize(stmt);
    }

    // If the distance is below a certain threshold, return the closest word
    if (minDistance < 3) { 
        return closestWord;
    } else {
        return ""; // No similar word found
    }
}

void ResponseVariator::addResponse(const std::string& topic, const std::string& response) {
    saveResponse(topic, response, 0.3f);  // Save the new response with default confidence
    neuralNet.train(topic, response);  // Train the neural network with the new input-output pair
}

void ResponseVariator::saveResponse(const std::string& topic, const std::string& response, float confidence) {
    const char* sql = R"(
        INSERT INTO responses (topic, response, confidence, use_count, created_at)
        VALUES (?, ?, ?, 1, datetime('now'))
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, topic.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, response.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, confidence);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

void ResponseVariator::updateConfidenceInDatabase(const std::string& input, const std::string& response, bool positive) {
    const char* updateQuery = 
        "UPDATE responses SET confidence = confidence + ? WHERE topic = ? AND response = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, updateQuery, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    double change = positive ? 0.1 : -0.1;  // Confidence change based on positive or negative feedback
    sqlite3_bind_double(stmt, 1, change);
    sqlite3_bind_text(stmt, 2, input.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, response.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to update confidence: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

std::string ResponseVariator::getFallbackResponse() const {
    return fallbackResponse;
}

void ResponseVariator::startTeachingMode() { teachingMode = true; }
void ResponseVariator::stopTeachingMode() { teachingMode = false; }
bool ResponseVariator::isTeaching() const { return teachingMode; }

std::vector<std::string> ResponseVariator::extractTopics(const std::string& input) {
    return WordVectorHelper::tokenize(input);
}

void ResponseVariator::teachAlternative(const std::string& topic, const std::string& altResponse) {
    saveResponse(topic, altResponse, 0.5f);  // Save alternative response with lower confidence
}

std::string ResponseVariator::getFollowupSuggestion() {
    // For simplicity, we can omit the follow-up suggestion logic unless needed
    return "";
}

void ResponseVariator::bulkTeachFromCSV(const std::string& filepath) {
    std::ifstream file(filepath);
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string topic, response, scoreStr;
        if (std::getline(ss, topic, ',') && std::getline(ss, response, ',') && std::getline(ss, scoreStr)) {
            float score = std::stof(scoreStr);
            saveResponse(topic, response, score);
            neuralNet.train(topic, response);  // Train the model with the new data
        }
    }
}


void ResponseVariator::trainFromDatabaseForDev() {
    std::cout << "Training the model from the database (for dev)..." << std::endl;

    // Train using the data in the database
    neuralNet.trainFromDatabase(db);

    // After training, save the model to a file
    saveModel();
    std::cout << "Training complete and model saved to file." << std::endl;
}



std::string ResponseVariator::generateResponseFromNN(const std::string& input) {
    // Vectorize the input (obtain its embedding)
    auto inputVec = neuralNet.vectorize(input);

    // Create a list to store candidate responses based on word similarity
    std::vector<std::pair<std::string, float>> candidateResponses;

    // Query all words from word_vectors table
    const char* sql = "SELECT word, vector FROM word_vectors;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string word = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* vecData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

            // Convert vector data from string to vector<float>
            std::vector<float> wordVec;
            std::istringstream vecStream(vecData);
            float val;
            while (vecStream >> val) {
                wordVec.push_back(val);
            }

            // Calculate cosine similarity between input and word vector
            float similarity = cosineSimilarity(inputVec, wordVec);
            candidateResponses.push_back({word, similarity});
        }
        sqlite3_finalize(stmt);
    }

    // Sort the candidate words by similarity (highest first)
    std::sort(candidateResponses.begin(), candidateResponses.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;  // Sort by similarity
    });

    // Generate a response by concatenating the most similar words
    std::string generatedResponse;
    int topN = 1;

    for (int i = 0; i < std::min(topN, (int)candidateResponses.size()); ++i) {
        generatedResponse += candidateResponses[i].first + " ";
    }

    // Check if no suitable response was generated
    if (generatedResponse.empty()) {
        return "I don't know yet.";
    }

    // Return the generated response
    return generatedResponse;
}


// Cosine similarity to measure the closeness between input and response embeddings
float ResponseVariator::cosineSimilarity(const std::vector<float>& vec1, const std::vector<float>& vec2) {
    float dotProduct = 0.0f;
    float norm1 = 0.0f;
    float norm2 = 0.0f;

    for (size_t i = 0; i < vec1.size(); ++i) {
        dotProduct += vec1[i] * vec2[i];
        norm1 += vec1[i] * vec1[i];
        norm2 += vec2[i] * vec2[i];
    }

    return dotProduct / (std::sqrt(norm1) * std::sqrt(norm2));  // Cosine similarity formula
}

double ResponseVariator::getConfidenceForResponse(const std::string& input, const std::string& response)
{
    const char* query =
        "SELECT confidence FROM responses WHERE topic = ? AND response = ? LIMIT 1;";
    sqlite3_stmt* stmt;
    double result = 0.5; // default mid confidence

    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, input.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, response.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return std::clamp(result, 0.0, 1.0);
}

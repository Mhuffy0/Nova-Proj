#pragma once
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <set>
#include <queue>
#include <deque>
#include <random>
#include <sqlite3.h>
#include "../Core/NeuralNet.hpp"
#include "../Core/WordVectorHelper.hpp"
#include "../Core/TopicExtractor.hpp"
#include "../Humanizer/ContextTracker.hpp"

class ResponseVariator {
public:
    ResponseVariator();

    void trainFromDatabaseOnce();  // Train from the database once
    void trainFromDatabaseForDev();  // Train for dev (when the database has grown large)
    void saveModel() {
        neuralNet.saveModelToFile("trained_model.txt");  // Save model
    }
    void loadModel() {
        neuralNet.loadModelFromFile("trained_model.txt");  // Load model
    }

    std::string getResponse(const std::string& input);
    void addResponse(const std::string& input, const std::string& response);
    void updateConfidenceInDatabase(const std::string& input, const std::string& response, bool positive);
    std::string getFallbackResponse() const;
    void startTeachingMode();
    void stopTeachingMode();
    bool isTeaching() const;
    std::vector<std::string> extractTopics(const std::string& input);
    void teachAlternative(const std::string& topic, const std::string& altResponse);
    std::string getFollowupSuggestion();
    void bulkTeachFromCSV(const std::string& filepath);
    std::unordered_map<std::string, std::pair<std::string, double>> knowledgeBase;
    NeuralNet neuralNet;
    void saveResponse(const std::string& input, const std::string& response, float confidence);
    double getConfidenceForResponse(const std::string& input, const std::string& response);

private:
    std::string findSimilarWord(const std::string& input);
    int levenshteinDistance(const std::string& a, const std::string& b);
    void loadDatabase();
    void createTablesIfNotExist();
    int turnCount = 0; 
    std::string lastUsedResponse;
    sqlite3* db = nullptr;
    std::map<std::string, std::set<std::string>> topicMap;
    std::set<std::string> askedQuestions;
    std::deque<std::string> contextMemory;
    std::default_random_engine rng;
    bool teachingMode = false;
    std::string fallbackResponse = "I don't know yet.";
    std::string lastTopic;
    ContextTracker contextTracker;

    void addToContext(const std::string& message);
    std::string summarizeContext() const;
    std::string lastFollowup = "";
    std::string generateResponseFromNN(const std::string& input);
    float cosineSimilarity(const std::vector<float>& vec1, const std::vector<float>& vec2);

};
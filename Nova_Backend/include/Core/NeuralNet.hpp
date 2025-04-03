#ifndef NEURALNET_HPP
#define NEURALNET_HPP

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <sqlite3.h>
#include <memory>

class NeuralNet {
public:
    NeuralNet();
    ~NeuralNet();


    std::unordered_map<std::string, std::vector<float>> wordEmbeddings;     
    std::vector<float> vectorize(const std::string& input);  // Vectorize input text into word vectors
    void reinforce(const std::string& input, const std::string& response);  // Reinforce learning
    void train(const std::string& input, const std::string& response);
    void ensureTable(sqlite3* db);  // Ensure necessary database tables exist
    void updateTokenVector(const std::string& token, const std::vector<float>& update);
    std::string generateResponse(const std::string& input);
    
    void trainFromDatabase(sqlite3* db);
    void saveModelToFile(const std::string& filename);  // Save model to file
    void loadModelFromFile(const std::string& filename);  // Load model from file
    float computeLoss(const std::vector<float>& predicted, const std::vector<float>& actual);
    std::vector<float> forwardPass(const std::vector<float>& input, const std::vector<float>& weights);
    void backpropagate(std::vector<float>& weights, const std::vector<float>& target, float loss, float learningRate);
    void trainNetwork(std::vector<std::vector<float>>& inputs, std::vector<std::vector<float>>& targets, std::vector<float>& weights, float learningRate, int epochs);
    std::unordered_map<std::string, std::vector<float>> responseEmbeddings;  // Store response embeddings
    void importModelToDatabase(const std::string& filename);

private:
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db;  // SQLite database connection
    void loadPretrainedEmbeddings(const std::string& filename);
    // std::vector<float> getRandomVector();  // Generate a random vector
    void storeTokenVector(const std::string& token, const std::vector<float>& vector);  // Store a token's vector
    std::vector<float> getTokenVector(const std::string& token);  // Retrieve vector for a token
    float cosineSimilarity(const std::vector<float>& vecA, const std::vector<float>& vecB);
    bool isTableEmpty(sqlite3* db);
    void backpropagate(std::vector<float>& weights, const std::vector<float>& input, const std::vector<float>& target, float learningRate);
    int embeddingSize = 3;  // Size of token embeddings (can be increased)
};

#endif // NEURALNET_HPP

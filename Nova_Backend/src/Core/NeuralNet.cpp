#include "../../include/Core/NeuralNet.hpp"
#include <sstream>
#include <cctype>
#include <map>
#include <algorithm>
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>

// Constructor: Initialize database connection and ensure necessary table
NeuralNet::NeuralNet() 
    : db(nullptr, sqlite3_close) {
    sqlite3* rawDb = nullptr;
    if (sqlite3_open("D:/Nova_Project/Nova_Backend/chatbot.db", &rawDb) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db.get()) << std::endl;
    }
    db.reset(rawDb);  // Use unique_ptr to manage db connection
    ensureTable(db.get());  // Ensure table exists
}

// Destructor: Automatically closes the database connection
NeuralNet::~NeuralNet() {}

// Ensure word_vectors table exists in the database
void NeuralNet::ensureTable(sqlite3* db) {
    const char* createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS word_vectors (
            word TEXT PRIMARY KEY,
            vector BLOB
        );
    )";

    char* errMessage;
    if (sqlite3_exec(db, createTableQuery, nullptr, nullptr, &errMessage) != SQLITE_OK) {
        std::cerr << "Error creating table: " << errMessage << std::endl;
        sqlite3_free(errMessage);
    } else {
        std::cout << "Table 'word_vectors' ensured in the database." << std::endl;
    }
}

// Check if the word_vectors table is empty
bool NeuralNet::isTableEmpty(sqlite3* db) {
    const char* sql = "SELECT COUNT(*) FROM word_vectors;";
    sqlite3_stmt* stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    return count == 0;  // If the count is 0, the table is empty
}

void NeuralNet::importModelToDatabase(const std::string& filename) {
    // Open the model file and import data as before
    std::ifstream modelFile(filename);
    if (!modelFile.is_open()) {
        std::cerr << "Error opening model file: " << filename << std::endl;
        return;
    }

    // Check if the table is empty before proceeding with import
    if (!isTableEmpty(db.get())) {
        std::cout << "Table 'word_vectors' is not empty. Skipping import." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(modelFile, line)) {
        std::istringstream lineStream(line);
        std::string word;
        std::vector<float> embedding;
        float value;

        // Read the word (first token)
        lineStream >> word;

        // Read the vector (embedding) values
        while (lineStream >> value) {
            embedding.push_back(value);
        }

        // If no embedding is found for the word, skip this line
        if (embedding.empty()) {
            continue;
        }

        // Convert embedding to string format for insertion
        std::ostringstream vectorStream;
        for (const auto& val : embedding) {
            vectorStream << val << " ";
        }
        std::string vectorData = vectorStream.str();

        // Insert word and its vector into the database
        const char* insertQuery = "INSERT OR REPLACE INTO word_vectors (word, vector) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db.get(), insertQuery, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db.get()) << std::endl;
            continue;
        }

        sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_STATIC);  // Bind word
        sqlite3_bind_text(stmt, 2, vectorData.c_str(), -1, SQLITE_STATIC);  // Bind vector data
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Failed to insert data for " << word << ": " << sqlite3_errmsg(db.get()) << std::endl;
        }
        sqlite3_finalize(stmt);
    }

    modelFile.close();
    std::cout << "Model imported into database successfully!" << std::endl;
}

std::vector<float> NeuralNet::vectorize(const std::string& input) {
    std::vector<float> embedding(embeddingSize, 0.0f);  // Initialize the embedding with zeros

    // Tokenize the input string by spaces
    std::istringstream tokenStream(input);
    std::string token;
    int wordCount = 0;  // Count the number of valid words in the input

    while (tokenStream >> token) {
        // Retrieve the embedding for this token (word)
        auto wordVector = getTokenVector(token);

        // Add this word's vector to the overall embedding
        for (int i = 0; i < embeddingSize; ++i) {
            embedding[i] += wordVector[i];
        }

        wordCount++;  // Increment the word count
    }

    // If we found any words, normalize the embedding by dividing by the word count
    if (wordCount > 0) {
        for (int i = 0; i < embeddingSize; ++i) {
            embedding[i] /= wordCount;
        }
    }

    // Normalize the vector (optional, for unit length or other transformations)
    float norm = 0.0f;
    for (float val : embedding) {
        norm += val * val;
    }
    norm = std::sqrt(norm);

    if (norm > 0) {
        for (float& val : embedding) {
            val /= norm;  // Normalize each element
        }
    }

    return embedding;
}

// Helper: Generate a random vector (for unseen words)
// std::vector<float> NeuralNet::getRandomVector() {
//     std::vector<float> randomVector(embeddingSize, 0.0f);
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_real_distribution<float> dist(0.0f, 1.0f);

//     for (int i = 0; i < embeddingSize; ++i) {
//         randomVector[i] = dist(gen);
//     }

//     return randomVector;
// }


std::unordered_map<std::string, std::vector<float>> pretrainedEmbeddings;

void NeuralNet::loadPretrainedEmbeddings(const std::string& filename) {
    std::ifstream file(filename);
    std::string word;
    while (file >> word) {
        std::vector<float> embedding(embeddingSize, 0.0f);
        for (int i = 0; i < embeddingSize; ++i) {
            file >> embedding[i];
        }
        pretrainedEmbeddings[word] = embedding;
    }
    std::cout << "Pre-trained embeddings loaded successfully!" << std::endl;
}

void NeuralNet::storeTokenVector(const std::string& token, const std::vector<float>& vector) {
    const char* sql = "INSERT OR REPLACE INTO word_vectors (word, vector) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    // Convert vector to a string for insertion
    std::ostringstream vecStream;
    for (float val : vector) {
        vecStream << val << " ";
    }
    std::string vectorData = vecStream.str();

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);  // Bind word
        sqlite3_bind_text(stmt, 2, vectorData.c_str(), -1, SQLITE_STATIC);  // Bind vector
        sqlite3_step(stmt);  // Execute the statement (store the vector)
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Error storing vector for " << token << ": " << sqlite3_errmsg(db.get()) << std::endl;
    }
}



void NeuralNet::reinforce(const std::string& input, const std::string& response) {
    auto inputVec = vectorize(input);
    auto responseVec = vectorize(response);

    // Example reinforcement: increase similarity between input and response
    for (int i = 0; i < embeddingSize; ++i) {
        inputVec[i] += 0.05f;
        responseVec[i] += 0.05f;
    }

    updateTokenVector(input, inputVec);
    updateTokenVector(response, responseVec);
}


void NeuralNet::saveModelToFile(const std::string& filename) {
    std::ofstream modelFile(filename, std::ios::app);  // Open file for appending
    
    if (!modelFile) {
        std::cerr << "Error opening file for saving model: " << filename << std::endl;
        return;
    }

    const char* sql = "SELECT topic, response FROM responses;";  // Query all responses from the database
    sqlite3_stmt* stmt;

    // Prepare the query to get responses from the 'responses' table
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare SQL query: " << sqlite3_errmsg(db.get()) << std::endl;
        return;
    }

    // Loop through each response from the database
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string topic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string response = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        // Vectorize the response
        std::vector<float> responseVector = vectorize(response);  // Get the vector for the response

        // Write the word and its vector to the file
        modelFile << response << " ";  // Write the response
        for (const auto& value : responseVector) {
            modelFile << value << " ";  // Write the vector elements
        }
        modelFile << std::endl;  // New line after each response

        std::cout << "Saved response: " << response << " with vector to file." << std::endl;
    }

    sqlite3_finalize(stmt);  // Clean up the prepared statement
    modelFile.close();  // Close the model file
    std::cout << "Model saved to " << filename << std::endl;
}
std::vector<float> NeuralNet::getTokenVector(const std::string& token) {
    // First, check in the pre-trained embeddings
    if (pretrainedEmbeddings.find(token) != pretrainedEmbeddings.end()) {
        return pretrainedEmbeddings[token];  // Return pre-trained embedding if available
    }

    // If not in pre-trained embeddings, check the database
    const char* sql = "SELECT vector FROM word_vectors WHERE word = ?;";
    sqlite3_stmt* stmt;
    std::vector<float> vector;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* vecData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string vecString(vecData);
            std::istringstream vecStream(vecString);
            float val;
            while (vecStream >> val) {
                vector.push_back(val);  // Push each value of the vector into the result
            }
        }
        sqlite3_finalize(stmt);
    }

    // If no vector is found in the database, return a zero vector or a default vector
    if (vector.empty()) {
        std::cout << "No embedding found for word: " << token << ". Returning default vector." << std::endl;
        return std::vector<float>(embeddingSize, 0.0f);  // Return a zero vector or any default vector
    }

    return vector;
}



void NeuralNet::trainFromDatabase(sqlite3* db) {
    const char* sql = "SELECT topic, response FROM responses;";  // SQL query to get input-response pairs
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string input = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));  // Get input (topic)
            std::string response = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));  // Get response

            // Train the model using the input-response pair
            train(input, response);  // Call the existing train method to update the model's embeddings
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Error querying database for training data: " << sqlite3_errmsg(db) << std::endl;
    }
}

void NeuralNet::loadModelFromFile(const std::string& filename) {
    std::ifstream inFile(filename);

    if (!inFile) {
        // If the file doesn't exist, notify the user and create an empty file
        std::cerr << "Model file does not exist, creating a new one!" << std::endl;
        
        // Create the model file and save an empty model
        saveModelToFile(filename);  // This will create an empty model file if it doesn't exist
        return;
    }

    // If file exists, proceed to load the model
    std::string word;
    std::string response;
    
    while (inFile >> word) {
        std::vector<float> vector;
        float val;
        while (inFile >> val) {
            vector.push_back(val);
        }
        wordEmbeddings[word] = vector;
    }

    // Load response embeddings if applicable
    while (inFile >> response) {
        std::vector<float> vector;
        float val;
        while (inFile >> val) {
            vector.push_back(val);
        }
        responseEmbeddings[response] = vector;
    }

    inFile.close();
    std::cout << "Model loaded from " << filename << std::endl;
}


void NeuralNet::updateTokenVector(const std::string& token, const std::vector<float>& vector) {
    wordEmbeddings[token] = vector;  // Update in memory
    
    // Now update the database with the new vector
    storeTokenVector(token, vector);  // Calls a helper function to store the vector in DB
}

// Cosine similarity function to calculate the similarity between two vectors
float NeuralNet::cosineSimilarity(const std::vector<float>& vecA, const std::vector<float>& vecB) {
    float dotProduct = 0.0f;
    float normA = 0.0f;
    float normB = 0.0f;

    for (size_t i = 0; i < vecA.size(); ++i) {
        dotProduct += vecA[i] * vecB[i];
        normA += vecA[i] * vecA[i];
        normB += vecB[i] * vecB[i];
    }

    normA = std::sqrt(normA);
    normB = std::sqrt(normB);

    if (normA == 0 || normB == 0) {
        return 0.0f;
    }

    return dotProduct / (normA * normB);
}

// Compute the loss function (Mean Squared Error)
float NeuralNet::computeLoss(const std::vector<float>& predicted, const std::vector<float>& actual) {
    float loss = 0.0f;
    for (size_t i = 0; i < predicted.size(); ++i) {
        loss += std::pow(predicted[i] - actual[i], 2);  // Squared difference
    }
    return loss / predicted.size();  // Return the mean of squared errors
}

// Perform forward pass (calculate output using the weights and input)
std::vector<float> NeuralNet::forwardPass(const std::vector<float>& input, const std::vector<float>& weights) {
    std::vector<float> output(input.size(), 0.0f);

    // Element-wise multiplication (dot product)
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = input[i] * weights[i];  // Weighted input
    }
    return output;  // Return the calculated output
}

void NeuralNet::backpropagate(std::vector<float>& weights, const std::vector<float>& target, float loss, float learningRate) {
    // Gradient descent: Calculate the gradient (simplified version)
    for (int i = 0; i < weights.size(); ++i) {
        // Compute the gradient: (predicted - target) * input for each weight
        float gradient = (weights[i] - target[i]) * weights[i]; // Simplified gradient calculation
        weights[i] -= learningRate * gradient; // Update the weight with the gradient descent step
    }
}



void NeuralNet::train(const std::string& input, const std::string& response) {
    // Vectorize the input and response to get their corresponding embeddings
    auto inputVec = vectorize(input);
    auto responseVec = vectorize(response);

    // Store initial embeddings of the response
    responseEmbeddings[response] = responseVec;

    // Forward pass: get the predicted output for the input
    auto predictedOutput = forwardPass(inputVec, inputVec);  // You may need to adjust weights here

    // Compute the loss based on predicted output and expected response embedding
    float loss = computeLoss(predictedOutput, responseVec);
    std::cout << "Initial Loss: " << loss << std::endl;

    backpropagate(inputVec, responseVec, loss, 0.01f);  // Example with learning rate = 0.01

    // Update response embeddings (we'll update after training)
    responseEmbeddings[response] = responseVec;  // Store updated embedding

    // Optionally, you can store the new word embeddings back into the database
    updateTokenVector(input, inputVec);  // Update input vector in the DB
    updateTokenVector(response, responseVec);  // Update response vector in the DB

    std::cout << "Training completed for input: " << input << " and response: " << response << std::endl;
}
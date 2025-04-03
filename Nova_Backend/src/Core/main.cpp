#include <iostream>
#include <string>
#include "../../include/Core/NeuralNet.hpp"
#include "../../include/Humanizer/ResponseVariator.hpp"

// Main function to run the chatbot
int main() {
    try {
        // Initialize NeuralNet and ResponseVariator
        NeuralNet neuralNet;
        ResponseVariator bot;

        // Load the pre-trained model if it exists
        std::string modelFile = "trained_model.txt";  // Specify your trained model file
        std::cout << "Loading model from: " << modelFile << std::endl;
        neuralNet.loadModelFromFile(modelFile);  // Load pre-trained embeddings into memory

        // Import model into database (if table is empty)
        neuralNet.importModelToDatabase(modelFile);  // Ensure word_vectors table is populated

        // Start the chatbot loop
        std::cout << "=== Nova AI Chat ===" << std::endl;
        std::cout << "Type 'exit' to quit the chat." << std::endl;

        while (true) {
            std::cout << "You: ";
            std::string input;
            std::getline(std::cin, input);

            // Check for exit command
            if (input == "exit") {
                std::cout << "Goodbye!" << std::endl;
                break;
            }

            // Get a response from the chatbot
            std::string response = bot.getResponse(input);
            std::cout << "Nova: " << response << std::endl;

            // Ask user for feedback on the response
            std::cout << "Was this response helpful? (y/n/skip): ";
            std::string feedback;
            std::getline(std::cin, feedback);

            // Handle feedback and update confidence in the database
            if (feedback == "y") {
                bot.updateConfidenceInDatabase(input, response, true);  // Positive feedback
            } else if (feedback == "n") {
                bot.updateConfidenceInDatabase(input, response, false);  // Negative feedback
            }
        }            
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

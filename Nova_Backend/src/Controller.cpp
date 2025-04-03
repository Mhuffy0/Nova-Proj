#include "../include/Controller.hpp"
#include <iostream>

ChatBotController::ChatBotController() {
    std::cout << "[BOOT] ChatBotController constructor called" << std::endl;
}
ChatBotController::~ChatBotController() {}

void ChatBotController::initialize(const std::string& modelFile) {
    std::cout << "[Controller] Loading model from: " << modelFile << std::endl;
    neuralNet.loadModelFromFile(modelFile);
    neuralNet.importModelToDatabase(modelFile);
}

std::string ChatBotController::getChatbotResponse(const std::string& input) {
    std::cout << "[Controller] Received input: " << input << std::endl;

    std::string response = bot.getResponse(input);
    std::cout << "[Controller] Response from bot: " << response << std::endl;

    if (response.empty() || response == input) {
        std::cout << "[Controller] Echo detected. Sending fallback." << std::endl;
        return "I'm not sure how to respond to that. Can you rephrase?";
    }

    return response;
}

void ChatBotController::provideFeedback(const std::string& input, const std::string& response, bool positive) {
    bot.updateConfidenceInDatabase(input, response, positive);
}

void ChatBotController::teachMode(const std::string& input) {
    size_t eq = input.find('=');
    if (eq == std::string::npos) {
        std::cout << "[TeachMode] Invalid input (missing '=')" << std::endl;
        return;
    }

    std::string topic = input.substr(0, eq);
    std::string response = input.substr(eq + 1);
    bot.addResponse(topic, response);
    std::cout << "[TeachMode] Taught: [" << topic << "] -> " << response << std::endl;
}

double ChatBotController::getConfidenceScore(const std::string& input, const std::string& response)
{
    return bot.getConfidenceForResponse(input, response);
}

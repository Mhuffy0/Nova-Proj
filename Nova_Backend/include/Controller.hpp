#pragma once

#include <string>
#include "Core/NeuralNet.hpp"
#include "Humanizer/ResponseVariator.hpp"

class ChatBotController {
public:
    ChatBotController();
    ~ChatBotController();

    void teachMode(const std::string& input);
    void initialize(const std::string& modelFile);
    std::string getChatbotResponse(const std::string& input);
    void provideFeedback(const std::string& input, const std::string& response, bool positive);
    double getConfidenceScore(const std::string& input, const std::string& response);

private:
    NeuralNet neuralNet;
    ResponseVariator bot;
};

#include "../../include/Humanizer/ContextTracker.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>


ContextTracker::ContextTracker() {}

void ContextTracker::addToContext(const std::string& message) {
    contextDeque.push_back(message);
    if (contextDeque.size() > MAX_CONTEXT_SIZE) {
        contextDeque.pop_front();
    }
}

std::deque<std::string> ContextTracker::getRecentMessages(size_t count) const {
    std::deque<std::string> recentMessages;
    for (size_t i = 0; i < count && i < contextDeque.size(); ++i) {
        recentMessages.push_back(contextDeque[contextDeque.size() - 1 - i]);
    }
    return recentMessages;
}


void ContextTracker::addMessage(const std::string& message) {
    addToContext(message);
}

std::string ContextTracker::getRecentContext() const {
    std::ostringstream oss;
    for (const auto& msg : contextDeque) {
        oss << msg << " ";
    }
    return oss.str();
}

std::string ContextTracker::summarizeContext() const {
    std::ostringstream oss;
    for (const auto& [topic, score] : topicRelevance) {
        if (score > 1) oss << topic << "(" << score << ") ";
    }
    return oss.str();
}

bool ContextTracker::topicExists(const std::string& topic) const {
    return topicRelevance.find(topic) != topicRelevance.end();
}

void ContextTracker::boostTopicRelevance(const std::string& topic, int amount) {
    topicRelevance[topic] += amount;
}

std::set<std::string> ContextTracker::extractKeywords(const std::string& message) const {
    std::set<std::string> keywords;
    std::istringstream iss(message);
    std::string word;
    while (iss >> word) {
        if (word.size() > 3) keywords.insert(word);
    }
    return keywords;
}
std::vector<std::string> ContextTracker::getRelevantContext() const {
    std::vector<std::string> relevantContext;
    for (const auto& msg : contextDeque) {
        relevantContext.push_back(msg);
    }
    return relevantContext;
}

void ContextTracker::boostTopicRelevanceByKeywords(const std::string& message) {
    auto keywords = extractKeywords(message);
    for (const auto& keyword : keywords) {
        boostTopicRelevance(keyword, 1); // boost topic relevance
    }
}
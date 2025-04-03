#pragma once
#include <string>
#include <deque>
#include <map>
#include <set>
#include <vector>
#include <sstream>

class ContextTracker {
public:
    ContextTracker();

    void addMessage(const std::string& message);
    std::deque<std::string> getRecentMessages(size_t count = 5) const;
    std::string getRecentContext() const;
    std::string summarizeContext() const;
    bool topicExists(const std::string& topic) const;
    void boostTopicRelevance(const std::string& topic, int amount);
    void boostTopicRelevanceByKeywords(const std::string& message);

    std::set<std::string> extractKeywords(const std::string& message) const; // Extract keywords
    std::vector<std::string> getRelevantContext() const; // Get weighted context
    void clearContext() {
        contextMessages.clear();
    }
private:
    std::vector<std::string> contextMessages;
    void addToContext(const std::string& message);
    std::deque<std::string> contextDeque;
    std::map<std::string, int> topicRelevance;
    static const size_t MAX_CONTEXT_SIZE = 5;
};

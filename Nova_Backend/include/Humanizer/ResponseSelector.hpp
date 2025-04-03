#pragma once
#include "../Core/NeuralNet.hpp"
#include <string>
#include <vector>
#include <tuple>
#include <sqlite3.h>

class ResponseSelector {
public:
    ResponseSelector(const std::string& dbPath);

    std::string chooseBest(const std::string& input);

private:
    void initializeDB();
    NeuralNet nn;
    sqlite3* db;
};
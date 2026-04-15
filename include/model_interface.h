#pragma once

#include <string>
#include <vector>

class ModelInterface {
public:
    ModelInterface();
    // Generate text given a prompt (synchronous stub)
    std::string generate(const std::string &prompt, int maxTokens = 128);
    // Produce a small deterministic embedding for demo purposes
    std::vector<float> embed(const std::string &text);
};

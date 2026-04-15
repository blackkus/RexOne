#pragma once

#include <string>
#include <vector>
#include <functional>

// Callback for streaming tokens
using TokenCallback = std::function<void(const std::string &token)>;

class ModelInterface {
public:
    ModelInterface();
    
    // Generate text given a prompt (synchronous)
    std::string generate(const std::string &prompt, int maxTokens = 128);
    
    // Generate with streaming callback (token-by-token)
    std::string generate_streaming(const std::string &prompt, int maxTokens, TokenCallback callback);
    
    // Produce a small deterministic embedding for demo purposes
    std::vector<float> embed(const std::string &text);
};

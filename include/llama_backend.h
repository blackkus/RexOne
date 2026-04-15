#pragma once

#include <string>
#include <vector>
#include <functional>

// Callback for streaming tokens during generation
using TokenCallback = std::function<void(const std::string &token)>;

class LlamaBackend {
public:
    LlamaBackend();
    ~LlamaBackend();

    // Initialize with a model path (GGUF format)
    // Returns true if successful
    bool initialize(const std::string &model_path);

    // Generate text with streaming callback
    // Calls token_callback for each token generated
    std::string generate(const std::string &prompt, int maxTokens, TokenCallback token_callback = nullptr);

    // Generate and collect all tokens into a single string
    std::string generate_blocking(const std::string &prompt, int maxTokens);

    // Check if backend is initialized
    bool is_initialized() const;

    // Get model info
    std::string model_info() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

// Global instance for demo simplicity
extern LlamaBackend g_llama_backend;

// Helper function to enable global backend
bool init_global_llama(const std::string &model_path);

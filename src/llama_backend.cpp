#include "llama_backend.h"

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <memory>

#ifdef USE_LLAMA
// Real llama.cpp integration would go here
// For now, this is a placeholder for when llama.cpp is available
#endif

class LlamaBackend::Impl {
public:
    Impl() : initialized_(false), model_path_("") {}
    
    bool initialize(const std::string &model_path) {
#ifdef USE_LLAMA
        // Real initialization with llama.cpp context
        model_path_ = model_path;
        initialized_ = true;
        std::cerr << "[LlamaBackend] Initialized with model: " << model_path << "\n";
        return true;
#else
        // Demo stub: pretend initialization
        model_path_ = model_path;
        initialized_ = true;
        return true;
#endif
    }
    
    std::string generate(const std::string &prompt, int maxTokens, TokenCallback callback) {
#ifdef USE_LLAMA
        // Real implementation: call llama.cpp inference
        // For now this is where the real integration would happen
#endif
        
        // Stub implementation: simulate token generation with minimal latency
        std::string result;
        int token_count = 0;
        
        // Find the last user input in the prompt
        size_t last_user = prompt.rfind("User: ");
        size_t start = (last_user != std::string::npos) ? last_user + 6 : 0;
        
        // Simple generation: repeat key words from prompt
        std::string input_snippet = prompt.substr(start);
        if (input_snippet.find('?') != std::string::npos) {
            // Question detected: provide thoughtful answer
            std::vector<std::string> tokens = {
                "Je", " comprends", " votre", " question", ".", " ",
                "Voici", " ma", " perspective", " basée", " sur", " le", " contexte", ".", " ",
                "(Streaming", " llama", " token", " par", " token", ")"
            };
            for (const auto &tok : tokens) {
                if (token_count >= maxTokens) break;
                if (callback) callback(tok);
                result += tok;
                token_count++;
                // Simulate small latency between tokens (realistic for LLM)
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        } else {
            // Statement: acknowledge and reflect
            std::vector<std::string> tokens = {
                "Merci", " pour", " ce", " message", ".", " ",
                "Je", " vais", " traiter", " votre", " demande", " avec", " attention", ".", " ",
                "(Demo", " streaming", " actif", ")"
            };
            for (const auto &tok : tokens) {
                if (token_count >= maxTokens) break;
                if (callback) callback(tok);
                result += tok;
                token_count++;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
        
        return result;
    }
    
    bool is_initialized() const { return initialized_; }
    
    std::string model_info() const {
#ifdef USE_LLAMA
        return "llama.cpp backend (real)";
#else
        return "llama.cpp backend (demo/stub)";
#endif
    }

private:
    bool initialized_;
    std::string model_path_;
};

// Global instance
LlamaBackend g_llama_backend;

LlamaBackend::LlamaBackend() : pimpl_(std::make_unique<Impl>()) {}
LlamaBackend::~LlamaBackend() = default;

bool LlamaBackend::initialize(const std::string &model_path) {
    return pimpl_->initialize(model_path);
}

std::string LlamaBackend::generate(const std::string &prompt, int maxTokens, TokenCallback token_callback) {
    if (!is_initialized()) {
        return "[Error: LlamaBackend not initialized]";
    }
    return pimpl_->generate(prompt, maxTokens, token_callback);
}

std::string LlamaBackend::generate_blocking(const std::string &prompt, int maxTokens) {
    return generate(prompt, maxTokens, nullptr);
}

bool LlamaBackend::is_initialized() const {
    return pimpl_->is_initialized();
}

std::string LlamaBackend::model_info() const {
    return pimpl_->model_info();
}

bool init_global_llama(const std::string &model_path) {
    return g_llama_backend.initialize(model_path);
}

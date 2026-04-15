#include "model_interface.h"
#include "llama_backend.h"

#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <cctype>
#include <algorithm>

ModelInterface::ModelInterface() {
    // Initialize global llama backend if available
    // In a real deployment, pass actual model path here
    (void)init_global_llama("models/gguf-model.bin");
}

std::string ModelInterface::generate(const std::string &prompt, int maxTokens) {
    // Use llama backend with streaming disabled
    if (g_llama_backend.is_initialized()) {
        return g_llama_backend.generate_blocking(prompt, maxTokens);
    }
    
    // Fallback: simple demo response
    std::string reply = "RexOne (demo): ";
    if (prompt.empty()) reply += "(no prompt)";
    else {
        if (prompt.find('?') != std::string::npos) reply += "Je vois une question. Voici une réponse courte.";
        else reply += "J'ai reçu votre message. (demo généré)";
    }
    if ((int)reply.size() > maxTokens) reply = reply.substr(0, maxTokens);
    return reply;
}

std::string ModelInterface::generate_streaming(const std::string &prompt, int maxTokens, TokenCallback callback) {
    // Use llama backend with streaming callback
    if (g_llama_backend.is_initialized()) {
        return g_llama_backend.generate(prompt, maxTokens, callback);
    }
    
    // Fallback: non-streaming demo
    return generate(prompt, maxTokens);
}

std::vector<float> ModelInterface::embed(const std::string &text) {
    const int D = 128;
    std::vector<float> v(D, 0.0f);
    if (text.empty()) {
        v[0] = 1.0f;
        return v;
    }

    std::string normalized;
    normalized.reserve(text.size());
    for (char ch : text) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        } else {
            normalized.push_back(' ');
        }
    }

    auto add_token = [&](const std::string &token, float weight) {
        size_t hash = std::hash<std::string>{}(token);
        int index = static_cast<int>(hash % D);
        v[index] += weight;
    };

    std::string token;
    for (char ch : normalized) {
        if (ch == ' ') {
            if (!token.empty()) {
                add_token(token, 1.0f + std::log1p(static_cast<float>(token.size())));
                if (token.size() >= 3) {
                    for (size_t j = 0; j + 3 <= token.size(); ++j) {
                        add_token(token.substr(j, 3), 0.5f);
                    }
                }
                token.clear();
            }
        } else {
            token.push_back(ch);
        }
    }
    if (!token.empty()) {
        add_token(token, 1.0f + std::log1p(static_cast<float>(token.size())));
        if (token.size() >= 3) {
            for (size_t j = 0; j + 3 <= token.size(); ++j) {
                add_token(token.substr(j, 3), 0.5f);
            }
        }
    }

    // Add lightweight character-level signal
    for (size_t i = 0; i < normalized.size(); ++i) {
        char ch = normalized[i];
        if (ch == ' ') continue;
        std::string single(1, ch);
        add_token(single, 0.15f);
    }

    float norm = 1e-9f;
    for (float x : v) {
        norm += x * x;
    }
    norm = std::sqrt(norm);
    for (float &x : v) {
        x /= norm;
    }
    return v;
}

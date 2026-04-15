#include "model_interface.h"
#include "llama_backend.h"

#include <string>
#include <vector>
#include <functional>
#include <cmath>

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
    const int D = 8;
    std::vector<float> v(D, 0.0f);
    std::hash<std::string> h;
    if (text.empty()) {
        v[0] = 1.0f;
    } else {
        for (size_t i = 0; i < text.size(); ++i) {
            // use small substrings to mix
            std::string s = text.substr(i, 3);
            size_t val = h(s);
            v[i % D] += static_cast<float>((val % 1000)) / 1000.0f;
        }
    }
    float norm = 1e-6f;
    for (float x : v) norm += x*x;
    norm = std::sqrt(norm);
    for (float &x : v) x /= norm;
    return v;
}

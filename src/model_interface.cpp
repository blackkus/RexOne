#include "model_interface.h"
#include "llama_backend.h"

#include <string>
#include <vector>
#include <functional>
#include <cmath>

ModelInterface::ModelInterface() {
    // If a local llama backend is available, initialize it (model path configurable later)
#ifdef USE_LLAMA
    // llama_init may be a no-op placeholder; real integration should pass a real model path
    (void)llama_init("models/ggml-model.bin");
#endif
}

std::string ModelInterface::generate(const std::string &prompt, int maxTokens) {
#ifdef USE_LLAMA
    // Delegate to llama backend when available
    std::string out = llama_generate(prompt, maxTokens);
    if (!out.empty()) return out;
    // fallback to demo below
#endif
    // Demo fallback: short deterministic reply
    std::string reply = "RexOne (demo): ";
    if (prompt.empty()) reply += "(no prompt)";
    else {
        if (prompt.find('?') != std::string::npos) reply += "Je vois une question. Voici une réponse courte.";
        else reply += "J'ai reçu votre message. (demo généré)";
    }
    if ((int)reply.size() > maxTokens) reply = reply.substr(0, maxTokens);
    return reply;
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

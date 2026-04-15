#include "orchestrator.h"

#include <string>
#include <sstream>

Orchestrator::Orchestrator(const std::string &dbpath)
    : model_(), mem_(dbpath) {}

std::vector<std::string> Orchestrator::get_recent_history() {
    return mem_.get_recent_history();
}

std::string Orchestrator::handle_user(const std::string &user_input) {
    // 1. Preprocess / store
    mem_.append_short(std::string("USER: ") + user_input);

    // 2. Embed query and retrieve facts from long-term (RAG)
    auto qemb = model_.embed(user_input);
    auto facts = mem_.retrieve_semantic(qemb, 5);

    // 3. Compose prompt
    std::ostringstream prompt;
    prompt << "[SYSTEM] You are RexOne, a compact AI assistant.\n";

    // Include recent history as-is (naive) until a soft char limit
    const size_t soft_limit = 3000; // chars
    for (const auto &h : get_recent_history()) {
        if (prompt.str().size() + h.size() > soft_limit) break;
        prompt << h << "\n";
    }

    if (!facts.empty()) {
        prompt << "\n[RETRIEVED CONTEXT]\n";
        for (const auto &f : facts) {
            prompt << "- " << f << "\n";
        }
    }

    prompt << "\nUser: " << user_input << "\nAssistant:";

    // 4. Call model to generate response
    std::string resp = model_.generate(prompt.str(), 512);

    // 5. Postprocess & update memories
    mem_.append_short(std::string("ASSISTANT: ") + resp);
    
    // Add to long-term memory with embedding for future retrieval
    std::string qa_pair = std::string("Q: ") + user_input + "\nA: " + resp;
    auto qa_embedding = model_.embed(qa_pair);
    mem_.add_longterm(qa_pair, qa_embedding);

    return resp;
}

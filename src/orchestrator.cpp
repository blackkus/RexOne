#include "orchestrator.h"

#include <string>
#include <sstream>

Orchestrator::Orchestrator(const std::string &dbpath)
    : model_(), mem_(dbpath), planner_(std::make_unique<Planner>()), autonomous_mode_(false) {}

Orchestrator::~Orchestrator() = default;

std::vector<std::string> Orchestrator::get_recent_history() {
    return mem_.get_recent_history();
}

std::vector<Planner::Action> Orchestrator::get_memory_management_candidates() {
    // Define candidate actions for autonomous memory management
    std::vector<Planner::Action> candidates;
    
    // Action 1: Clean up old memories (low cost, low risk, high benefit)
    Planner::Action cleanup;
    cleanup.name = "cleanup_old_memories";
    cleanup.description = "Remove memories older than 30 days to free space";
    cleanup.cost = 0.1;
    cleanup.risk = 0.1;
    cleanup.benefit = 0.7;
    cleanup.requires_approval = false;
    candidates.push_back(cleanup);
    
    // Action 2: Compress context (medium cost, low risk, medium benefit)
    Planner::Action compress;
    compress.name = "compress_context";
    compress.description = "Summarize long-term memories to reduce storage";
    compress.cost = 0.3;
    compress.risk = 0.2;
    compress.benefit = 0.5;
    compress.requires_approval = false;
    candidates.push_back(compress);
    
    // Action 3: Export and backup (low cost, very low risk, high benefit)
    Planner::Action backup;
    backup.name = "export_backup";
    backup.description = "Create encrypted backup of all memories";
    backup.cost = 0.15;
    backup.risk = 0.0;
    backup.benefit = 0.8;
    backup.requires_approval = false;
    candidates.push_back(backup);
    
    // Action 4: Do nothing (no cost, no risk, no benefit)
    Planner::Action idle;
    idle.name = "idle";
    idle.description = "Continue normal operation without action";
    idle.cost = 0.0;
    idle.risk = 0.0;
    idle.benefit = 0.0;
    idle.requires_approval = false;
    candidates.push_back(idle);

    return candidates;
}

Planner::Decision Orchestrator::make_decision(const std::vector<Planner::Action>& candidates) {
    if (!planner_) {
        return Planner::Decision{};
    }
    return planner_->plan(candidates, "user_context");
}

void Orchestrator::execute_decision(const Planner::Decision& decision) {
    // Placeholder for executing autonomous decisions
    // In the future, this would actually perform the action
    // (e.g., call cleanup_old_memories(), export_backup(), etc.)
    if (decision.approved) {
        std::string action_name = decision.chosen_action.name;
        // Log that it was executed (for now, just a stub)
    }
}

std::vector<std::string> Orchestrator::get_decisions_log() const {
    if (!planner_) {
        return {};
    }
    return planner_->get_audit_log();
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
    
    // 6. Autonomous mode: periodically evaluate memory management decisions
    if (autonomous_mode_) {
        auto candidates = get_memory_management_candidates();
        auto decision = make_decision(candidates);
        execute_decision(decision);
    }

    return resp;
}

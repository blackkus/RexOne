#pragma once

#include <string>
#include <vector>
#include <deque>
#include <mutex>

class MemoryManager {
public:
    explicit MemoryManager(const std::string &dbpath);

    // append to short-term history (thread-safe)
    void append_short(const std::string &utter);

    // retrieve k textual 'facts' by semantic similarity (stubbed deterministic behavior)
    std::vector<std::string> retrieve_semantic(const std::vector<float> &query, int k);

    // add to long-term store (stubbed)
    void add_longterm(const std::string &text);

    // approximate total tokens in short history
    int total_tokens_estimate();

    // expose recent history
    std::vector<std::string> get_recent_history();

private:
    std::deque<std::string> short_history_;
    std::mutex mu_;
    size_t short_window_tokens_ = 2048; // approximate token budget
};

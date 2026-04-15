#include "memory_manager.h"

#include <algorithm>
#include <cctype>

MemoryManager::MemoryManager(const std::string &dbpath) {
    (void)dbpath; // stub: no persistent DB in demo
}

void MemoryManager::append_short(const std::string &utter) {
    std::lock_guard<std::mutex> lk(mu_);
    short_history_.push_back(utter);
    // simple trimming by approximate token count using whitespace heuristic
    while (total_tokens_estimate() > static_cast<int>(short_window_tokens_) && !short_history_.empty()) {
        short_history_.pop_front();
    }
}

std::vector<std::string> MemoryManager::retrieve_semantic(const std::vector<float> & /*query*/, int k) {
    std::lock_guard<std::mutex> lk(mu_);
    std::vector<std::string> out;
    // naive: return the last k items from short_history_
    for (int i = static_cast<int>(short_history_.size()) - 1; i >= 0 && (int)out.size() < k; --i) {
        out.push_back(short_history_[i]);
    }
    std::reverse(out.begin(), out.end());
    return out;
}

void MemoryManager::add_longterm(const std::string &text) {
    std::lock_guard<std::mutex> lk(mu_);
    // For demo, append to short history as well
    short_history_.push_back(std::string("[LONG] ") + text);
}

int MemoryManager::total_tokens_estimate() {
    // crude estimate: number of words across history
    int total = 0;
    for (const auto &s : short_history_) {
        int words = 0;
        bool inw = false;
        for (char c : s) {
            if (std::isspace(static_cast<unsigned char>(c))) { if (inw) { words++; inw = false; } }
            else inw = true;
        }
        if (inw) words++;
        total += words;
    }
    return total;
}

std::vector<std::string> MemoryManager::get_recent_history() {
    std::lock_guard<std::mutex> lk(mu_);
    std::vector<std::string> out(short_history_.begin(), short_history_.end());
    return out;
}

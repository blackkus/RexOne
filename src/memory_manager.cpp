#include "memory_manager.h"
#include "vector_store.h"

#include <algorithm>
#include <cctype>

MemoryManager::MemoryManager(const std::string &dbpath)
    : vector_store_(std::make_unique<VectorStore>(8, 10000)) {
    (void)dbpath; // stub: no persistent DB in demo, vector store is in-memory
}

MemoryManager::~MemoryManager() = default;

void MemoryManager::append_short(const std::string &utter) {
    std::lock_guard<std::mutex> lk(mu_);
    short_history_.push_back(utter);
    // simple trimming by approximate token count using whitespace heuristic
    while (total_tokens_estimate() > static_cast<int>(short_window_tokens_) && !short_history_.empty()) {
        short_history_.pop_front();
    }
}

std::vector<std::string> MemoryManager::retrieve_semantic(const std::vector<float> &query, int k) {
    std::lock_guard<std::mutex> lk(mu_);
    std::vector<std::string> out;
    
    if (!vector_store_) return out;
    
    auto entries = vector_store_->search(query, k);
    for (const auto &entry : entries) {
        out.push_back(entry.text);
    }
    return out;
}

void MemoryManager::add_longterm(const std::string &text, const std::vector<float> &embedding) {
    std::lock_guard<std::mutex> lk(mu_);
    if (!vector_store_) return;
    
    // Generate ID based on vector store size
    std::string id = "mem_" + std::to_string(vector_store_->size());
    vector_store_->add(id, text, embedding);
}

int MemoryManager::total_tokens_estimate() {
    // crude estimate: number of words across history
    int total = 0;
    for (const auto &s : short_history_) {
        int words = 0;
        bool inw = false;
        for (char c : s) {
            if (std::isspace(static_cast<unsigned char>(c))) { 
                if (inw) { words++; inw = false; } 
            }
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

VectorStore* MemoryManager::get_vector_store() const {
    return vector_store_.get();
}

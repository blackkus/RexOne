#pragma once

#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <memory>

class VectorStore;

class MemoryManager {
public:
    explicit MemoryManager(const std::string &dbpath);
    ~MemoryManager();

    // append to short-term history (thread-safe)
    void append_short(const std::string &utter);

    // retrieve k textual 'facts' by semantic similarity from long-term store
    std::vector<std::string> retrieve_semantic(const std::vector<float> &query, int k);

    // add to long-term store with embedding
    void add_longterm(const std::string &text, const std::vector<float> &embedding);

    // approximate total tokens in short history
    int total_tokens_estimate();

    // expose recent history
    std::vector<std::string> get_recent_history();

    // get pointer to vector store (for external management)
    VectorStore* get_vector_store() const;

private:
    std::deque<std::string> short_history_;
    std::mutex mu_;
    size_t short_window_tokens_ = 2048; // approximate token budget
    std::unique_ptr<VectorStore> vector_store_;
};

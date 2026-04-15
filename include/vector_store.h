#pragma once

#include <string>
#include <vector>
#include <map>

struct VectorStoreEntry {
    std::string id;
    std::string text;
    std::vector<float> embedding;
};

class VectorStore {
public:
    VectorStore(int dimension = 8, int maxElements = 10000);
    ~VectorStore();

    // Add a vector entry to the store
    void add(const std::string &id, const std::string &text, const std::vector<float> &embedding);

    // Search for k nearest neighbors using cosine similarity
    std::vector<VectorStoreEntry> search(const std::vector<float> &query, int k = 5);

    // Get total size
    size_t size() const;

    // Clear store
    void clear();

private:
    int dimension_;
    int nextLabel_;
    std::map<int, VectorStoreEntry> entries_; // label -> entry
};

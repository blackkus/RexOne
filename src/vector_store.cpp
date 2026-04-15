#include "vector_store.h"

#include <cmath>
#include <algorithm>

VectorStore::VectorStore(int dimension, int maxElements)
    : dimension_(dimension), nextLabel_(0) {
    // hnswlib integration can be added here in future
    // For now, using efficient naive cosine similarity
}

VectorStore::~VectorStore() = default;

void VectorStore::add(const std::string &id, const std::string &text, const std::vector<float> &embedding) {
    if ((int)embedding.size() != dimension_) {
        return; // Dimension mismatch
    }
    
    int label = nextLabel_++;
    entries_[label] = VectorStoreEntry{id, text, embedding};
}

std::vector<VectorStoreEntry> VectorStore::search(const std::vector<float> &query, int k) {
    std::vector<VectorStoreEntry> result;

    // Cosine similarity search (efficient for small memory footprints)
    auto dot = [](const std::vector<float> &a, const std::vector<float> &b) {
        float s = 0.0f;
        size_t n = std::min(a.size(), b.size());
        for (size_t i = 0; i < n; ++i) s += a[i] * b[i];
        return s;
    };

    auto norm = [](const std::vector<float> &a) {
        float s = 0.0f;
        for (float v : a) s += v * v;
        return std::sqrt(s);
    };

    struct Scored {
        int label;
        float score;
    };

    std::vector<Scored> scores;
    float qnorm = norm(query);
    
    for (const auto &[label, entry] : entries_) {
        float enorm = norm(entry.embedding);
        float denom = qnorm * enorm;
        float sc = (denom > 1e-6f) ? dot(query, entry.embedding) / denom : 0.0f;
        scores.push_back({label, sc});
    }

    // Sort by score descending
    std::sort(scores.begin(), scores.end(), 
              [](const Scored &a, const Scored &b) { return a.score > b.score; });

    // Return top k results
    for (int i = 0; i < k && i < (int)scores.size(); ++i) {
        if (entries_.count(scores[i].label)) {
            result.push_back(entries_[scores[i].label]);
        }
    }

    return result;
}

size_t VectorStore::size() const {
    return entries_.size();
}

void VectorStore::clear() {
    entries_.clear();
    nextLabel_ = 0;
}

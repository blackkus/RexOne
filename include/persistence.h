#pragma once

#include <string>
#include <vector>
#include <memory>

struct MemoryRecord {
    std::string id;
    std::string text;
    std::vector<float> embedding;
    long long timestamp;  // Unix timestamp
    float importance;     // 0.0 to 1.0
};

class Persistence {
public:
    // Initialize database at given path
    explicit Persistence(const std::string &db_path);
    ~Persistence();

    // Open/create database
    bool open();

    // Close database
    void close();

    // Check if database is connected
    bool is_connected() const;

    // Save a memory record (insert or update)
    bool save_memory(const MemoryRecord &record);

    // Load all memories from database
    std::vector<MemoryRecord> load_all_memories();

    // Load memories after a certain timestamp
    std::vector<MemoryRecord> load_memories_since(long long timestamp);

    // Delete old memories (older than timestamp)
    bool cleanup_old_memories(long long keep_since);

    // Get database statistics
    int get_memory_count() const;

    // Clear all memories (be careful!)
    bool clear_all();

private:
    std::string db_path_;
    class Impl;
    std::unique_ptr<Impl> pimpl_;

    // Serialize/deserialize embedding vectors to/from string
    static std::string serialize_embedding(const std::vector<float> &emb);
    static std::vector<float> deserialize_embedding(const std::string &str);
};

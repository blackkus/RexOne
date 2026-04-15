#include "persistence.h"

#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>

#ifdef USE_SQLITE
#include <sqlite3.h>
#endif

class Persistence::Impl {
public:
    Impl(const std::string &db_path) : db_path_(db_path), connected_(false) {
#ifdef USE_SQLITE
        db_ = nullptr;
#endif
    }

    ~Impl() {
        close();
    }

    bool open() {
#ifdef USE_SQLITE
        if (sqlite3_open(db_path_.c_str(), &db_) == SQLITE_OK) {
            // Create table if it doesn't exist
            const char *create_table_sql = R"(
                CREATE TABLE IF NOT EXISTS memories (
                    id TEXT PRIMARY KEY,
                    text TEXT NOT NULL,
                    embedding TEXT,
                    timestamp INTEGER,
                    importance REAL
                );
                CREATE INDEX IF NOT EXISTS idx_timestamp ON memories(timestamp);
            )";
            char *err = nullptr;
            if (sqlite3_exec(db_, create_table_sql, nullptr, nullptr, &err) == SQLITE_OK) {
                connected_ = true;
                return true;
            } else {
                if (err) sqlite3_free(err);
                close();
                return false;
            }
        }
        return false;
#else
        std::cerr << "[Persistence] SQLite not available (USE_SQLITE=OFF)\n";
        connected_ = false;
        return false;
#endif
    }

    bool close() {
#ifdef USE_SQLITE
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
#endif
        connected_ = false;
        return true;
    }

    bool is_connected() const { return connected_; }

    bool save_memory(const MemoryRecord &record) {
#ifdef USE_SQLITE
        if (!connected_) return false;

        std::string emb_str = Persistence::serialize_embedding(record.embedding);
        
        const char *sql = R"(
            INSERT OR REPLACE INTO memories (id, text, embedding, timestamp, importance)
            VALUES (?, ?, ?, ?, ?)
        )";
        
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_text(stmt, 1, record.id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, record.text.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, emb_str.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 4, record.timestamp);
        sqlite3_bind_double(stmt, 5, record.importance);

        bool result = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return result;
#else
        (void)record;
        return false;
#endif
    }

    std::vector<MemoryRecord> load_all_memories() {
#ifdef USE_SQLITE
        std::vector<MemoryRecord> records;
        if (!connected_) return records;

        const char *sql = "SELECT id, text, embedding, timestamp, importance FROM memories ORDER BY timestamp DESC";
        sqlite3_stmt *stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return records;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            MemoryRecord rec;
            rec.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            rec.text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            const char *emb_str = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            if (emb_str) {
                rec.embedding = Persistence::deserialize_embedding(std::string(emb_str));
            }
            rec.timestamp = sqlite3_column_int64(stmt, 3);
            rec.importance = sqlite3_column_double(stmt, 4);
            records.push_back(rec);
        }

        sqlite3_finalize(stmt);
        return records;
#else
        return std::vector<MemoryRecord>();
#endif
    }

    std::vector<MemoryRecord> load_memories_since(long long timestamp) {
#ifdef USE_SQLITE
        std::vector<MemoryRecord> records;
        if (!connected_) return records;

        sqlite3_stmt *stmt;
        const char *sql = "SELECT id, text, embedding, timestamp, importance FROM memories WHERE timestamp >= ? ORDER BY timestamp DESC";
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return records;
        }

        sqlite3_bind_int64(stmt, 1, timestamp);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            MemoryRecord rec;
            rec.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            rec.text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            const char *emb_str = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            if (emb_str) {
                rec.embedding = Persistence::deserialize_embedding(std::string(emb_str));
            }
            rec.timestamp = sqlite3_column_int64(stmt, 3);
            rec.importance = sqlite3_column_double(stmt, 4);
            records.push_back(rec);
        }

        sqlite3_finalize(stmt);
        return records;
#else
        (void)timestamp;
        return std::vector<MemoryRecord>();
#endif
    }

    bool cleanup_old_memories(long long keep_since) {
#ifdef USE_SQLITE
        if (!connected_) return false;

        sqlite3_stmt *stmt;
        const char *sql = "DELETE FROM memories WHERE timestamp < ?";
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int64(stmt, 1, keep_since);
        bool result = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return result;
#else
        (void)keep_since;
        return false;
#endif
    }

    int get_memory_count() const {
#ifdef USE_SQLITE
        if (!connected_) return 0;

        sqlite3_stmt *stmt;
        const char *sql = "SELECT COUNT(*) FROM memories";
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return 0;
        }

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return count;
#else
        return 0;
#endif
    }

    bool clear_all() {
#ifdef USE_SQLITE
        if (!connected_) return false;

        const char *sql = "DELETE FROM memories";
        char *err = nullptr;
        
        bool result = (sqlite3_exec(db_, sql, nullptr, nullptr, &err) == SQLITE_OK);
        if (err) sqlite3_free(err);
        return result;
#else
        return false;
#endif
    }

private:
#ifdef USE_SQLITE
    sqlite3 *db_;
#endif
    std::string db_path_;
    bool connected_;
};

// Public Persistence implementation

Persistence::Persistence(const std::string &db_path)
    : db_path_(db_path), pimpl_(std::make_unique<Impl>(db_path)) {}

Persistence::~Persistence() = default;

bool Persistence::open() {
    return pimpl_->open();
}

void Persistence::close() {
    pimpl_->close();
}

bool Persistence::is_connected() const {
    return pimpl_->is_connected();
}

bool Persistence::save_memory(const MemoryRecord &record) {
    return pimpl_->save_memory(record);
}

std::vector<MemoryRecord> Persistence::load_all_memories() {
    return pimpl_->load_all_memories();
}

std::vector<MemoryRecord> Persistence::load_memories_since(long long timestamp) {
    return pimpl_->load_memories_since(timestamp);
}

bool Persistence::cleanup_old_memories(long long keep_since) {
    return pimpl_->cleanup_old_memories(keep_since);
}

int Persistence::get_memory_count() const {
    return pimpl_->get_memory_count();
}

bool Persistence::clear_all() {
    return pimpl_->clear_all();
}

std::string Persistence::serialize_embedding(const std::vector<float> &emb) {
    std::ostringstream oss;
    for (size_t i = 0; i < emb.size(); ++i) {
        if (i > 0) oss << ",";
        oss << emb[i];
    }
    return oss.str();
}

std::vector<float> Persistence::deserialize_embedding(const std::string &str) {
    std::vector<float> emb;
    if (str.empty()) return emb;
    
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        try {
            emb.push_back(std::stof(token));
        } catch (...) {
            // Skip malformed entries
        }
    }
    return emb;
}

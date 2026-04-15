#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

struct VS_Entry {
    std::string id;
    std::string text;
    std::vector<float> emb;
};

static std::vector<VS_Entry> vs_db;

void vs_add(const std::string &id, const std::string &text, const std::vector<float> &emb) {
    vs_db.push_back(VS_Entry{id, text, emb});
}

static float dot(const std::vector<float> &a, const std::vector<float> &b) {
    float s = 0.0f;
    size_t n = std::min(a.size(), b.size());
    for (size_t i = 0; i < n; ++i) s += a[i]*b[i];
    return s;
}

static float norm(const std::vector<float> &a) {
    float s = 0.0f;
    for (float v : a) s += v*v;
    return std::sqrt(s);
}

std::vector<std::string> vs_search(const std::vector<float> &q, int k) {
    struct Sc { float score; int idx; };
    std::vector<Sc> scores;
    for (size_t i = 0; i < vs_db.size(); ++i) {
        float denom = norm(vs_db[i].emb) * norm(q);
        float sc = (denom > 1e-6f) ? dot(vs_db[i].emb, q) / denom : 0.0f;
        scores.push_back({sc, (int)i});
    }
    std::sort(scores.begin(), scores.end(), [](const Sc &a, const Sc &b){ return a.score > b.score; });
    std::vector<std::string> out;
    for (int i = 0; i < k && i < (int)scores.size(); ++i) {
        out.push_back(vs_db[scores[i].idx].text);
    }
    return out;
}

#pragma once

#include <string>

class Tokenizer {
public:
    Tokenizer();
    // approximate token count (whitespace-based) for demo
    int tokens(const std::string &s) const;
    // encode prompt for model (identity stub for demo)
    std::string encode_for_model(const std::string &prompt) const;
};

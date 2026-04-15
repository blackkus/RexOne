#include "tokenizer.h"

#include <sstream>

Tokenizer::Tokenizer() {}

int Tokenizer::tokens(const std::string &s) const {
    // crude whitespace-based token estimate
    std::istringstream iss(s);
    int count = 0;
    std::string tok;
    while (iss >> tok) ++count;
    return count;
}

std::string Tokenizer::encode_for_model(const std::string &prompt) const {
    // identity stub for demo; real implementation should use a BPE tokenizer
    return prompt;
}

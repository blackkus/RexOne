#pragma once

#include <string>

#ifdef USE_LLAMA
bool llama_init(const std::string &model_path);
std::string llama_generate(const std::string &prompt, int maxTokens);
#else
// stubs when backend disabled
inline bool llama_init(const std::string &){ return false; }
inline std::string llama_generate(const std::string &){ return std::string("[llama disabled]"); }
#endif

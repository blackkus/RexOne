#include "llama_backend.h"

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <memory>
#include <vector>
#include <algorithm>

#ifdef USE_LLAMA
#include <llama.h>
#endif

class LlamaBackend::Impl {
public:
    Impl()
        : initialized_(false), model_path_("")
#ifdef USE_LLAMA
        , model_(nullptr), ctx_(nullptr), sampler_(nullptr)
#endif
    {}
    ~Impl() {
#ifdef USE_LLAMA
        if (sampler_) {
            llama_sampler_free(sampler_);
            sampler_ = nullptr;
        }
        if (ctx_) {
            llama_free(ctx_);
            ctx_ = nullptr;
        }
        if (model_) {
            llama_model_free(model_);
            model_ = nullptr;
        }
        llama_backend_free();
#endif
    }
    
    bool initialize(const std::string &model_path) {
#ifdef USE_LLAMA
        model_path_ = model_path;
        llama_backend_init();

        llama_model_params mparams = llama_model_default_params();
        mparams.use_mmap = true;
        mparams.use_mlock = false;
        mparams.check_tensors = false;
        mparams.n_gpu_layers = 0;
        mparams.main_gpu = -1;
        mparams.vocab_only = false;

        model_ = llama_model_load_from_file(model_path.c_str(), mparams);
        if (!model_) {
            std::cerr << "[LlamaBackend] Failed to load model from: " << model_path << std::endl;
            return false;
        }

        llama_context_params cparams = llama_context_default_params();
        cparams.n_ctx = 2048;
        cparams.n_batch = 8;
        cparams.n_threads = 0;
        cparams.n_threads_batch = 0;
        cparams.embeddings = false;
        cparams.n_seq_max = 1;

        ctx_ = llama_init_from_model(model_, cparams);
        if (!ctx_) {
            std::cerr << "[LlamaBackend] Failed to initialize context for model: " << model_path << std::endl;
            llama_model_free(model_);
            model_ = nullptr;
            return false;
        }

        llama_sampler_chain_params sparams = llama_sampler_chain_default_params();
        sampler_ = llama_sampler_chain_init(sparams);
        if (sampler_) {
            llama_sampler_chain_add(sampler_, llama_sampler_init_temp(0.8f));
            llama_sampler_chain_add(sampler_, llama_sampler_init_top_k(40));
            llama_sampler_chain_add(sampler_, llama_sampler_init_top_p(0.95f, 1));
            llama_sampler_chain_add(sampler_, llama_sampler_init_greedy());
            llama_set_sampler(ctx_, 0, sampler_);
        }

        initialized_ = true;
        std::cerr << "[LlamaBackend] Initialized with llama.cpp model: " << model_path << "\n";
        return true;
#else
        model_path_ = model_path;
        initialized_ = true;
        return true;
#endif
    }
    
    std::string generate(const std::string &prompt, int maxTokens, TokenCallback callback) {
#ifdef USE_LLAMA
        if (!ctx_ || !model_) {
            return "[Error: Llama backend not initialized]";
        }

        const llama_vocab * vocab = llama_model_get_vocab(llama_get_model(ctx_));
        if (!vocab) {
            return "[Error: failed to access llama vocabulary]";
        }

        int32_t token_capacity = static_cast<int32_t>(prompt.size() * 4 + 16);
        std::vector<llama_token> tokens(token_capacity);
        int32_t n_tokens = llama_tokenize(vocab, prompt.c_str(), static_cast<int32_t>(prompt.size()), tokens.data(), token_capacity, true, false);
        if (n_tokens < 0) {
            return "[Error: tokenization failed]";
        }
        tokens.resize(n_tokens);

        if (n_tokens > 0) {
            llama_batch prompt_batch = llama_batch_get_one(tokens.data(), n_tokens);
            int32_t result = llama_decode(ctx_, prompt_batch);
            if (result != 0) {
                return "[Error: llama_decode failed during prompt processing]";
            }
        }

        std::string result;
        result.reserve(maxTokens * 5);
        for (int i = 0; i < maxTokens; ++i) {
            llama_token token_id = llama_sampler_sample(sampler_, ctx_, -1);
            if (token_id == LLAMA_TOKEN_NULL) {
                break;
            }
            llama_sampler_accept(sampler_, token_id);

            if (llama_vocab_is_eog(vocab, token_id) || token_id == llama_vocab_eos(vocab)) {
                break;
            }

            char buffer[256] = {};
            int32_t text_len = llama_token_to_piece(vocab, token_id, buffer, sizeof(buffer), 0, false);
            if (text_len < 0) {
                break;
            }
            std::string token_text(buffer, std::max(0, text_len));
            result += token_text;
            if (callback) {
                callback(token_text);
            }

            llama_batch token_batch = llama_batch_get_one(&token_id, 1);
            int32_t result_code = llama_decode(ctx_, token_batch);
            if (result_code != 0) {
                break;
            }
        }

        return result;
#else
        std::string result;
        int token_count = 0;
        
        size_t last_user = prompt.rfind("User: ");
        size_t start = (last_user != std::string::npos) ? last_user + 6 : 0;
        
        std::string input_snippet = prompt.substr(start);
        if (input_snippet.find('?') != std::string::npos) {
            std::vector<std::string> tokens = {
                "Je", " comprends", " votre", " question", ".", " ",
                "Voici", " ma", " perspective", " basée", " sur", " le", " contexte", ".", " ",
                "(Streaming", " llama", " token", " par", " token", ")"
            };
            for (const auto &tok : tokens) {
                if (token_count >= maxTokens) break;
                if (callback) callback(tok);
                result += tok;
                token_count++;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        } else {
            std::vector<std::string> tokens = {
                "Merci", " pour", " ce", " message", ".", " ",
                "Je", " vais", " traiter", " votre", " demande", " avec", " attention", ".", " ",
                "(Demo", " streaming", " actif", ")"
            };
            for (const auto &tok : tokens) {
                if (token_count >= maxTokens) break;
                if (callback) callback(tok);
                result += tok;
                token_count++;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
        
        return result;
#endif
    }
    
    bool is_initialized() const { return initialized_; }
    
    std::string model_info() const {
#ifdef USE_LLAMA
        if (model_) {
            char buf[256] = {};
            llama_model_desc(model_, buf, sizeof(buf));
            return std::string("llama.cpp backend: ") + buf;
        }
        return "llama.cpp backend (uninitialized)";
#else
        return "llama.cpp backend (demo/stub)";
#endif
    }

private:
    bool initialized_;
    std::string model_path_;
#ifdef USE_LLAMA
    struct llama_model *model_;
    struct llama_context *ctx_;
    struct llama_sampler *sampler_;
#endif
};

// Global instance
LlamaBackend g_llama_backend;

LlamaBackend::LlamaBackend() : pimpl_(std::make_unique<Impl>()) {}
LlamaBackend::~LlamaBackend() = default;

bool LlamaBackend::initialize(const std::string &model_path) {
    return pimpl_->initialize(model_path);
}

std::string LlamaBackend::generate(const std::string &prompt, int maxTokens, TokenCallback token_callback) {
    if (!is_initialized()) {
        return "[Error: LlamaBackend not initialized]";
    }
    return pimpl_->generate(prompt, maxTokens, token_callback);
}

std::string LlamaBackend::generate_blocking(const std::string &prompt, int maxTokens) {
    return generate(prompt, maxTokens, nullptr);
}

bool LlamaBackend::is_initialized() const {
    return pimpl_->is_initialized();
}

std::string LlamaBackend::model_info() const {
    return pimpl_->model_info();
}

bool init_global_llama(const std::string &model_path) {
    return g_llama_backend.initialize(model_path);
}

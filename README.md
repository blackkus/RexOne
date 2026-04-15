# RexOne - Compact AI Agent Orchestrator

RexOne is a modern AI agent orchestrator written in **C++17** for performance, memory efficiency, and autonomous decision-making. Designed to fit in a pocket-sized device.

## Features

- **Compact Orchestrator**: Manages context, memory, and inference pipelines efficiently
- **Multi-tier Memory Management**: Short-term (sliding window), mid-term (summaries), long-term (vector-based retrieval)
- **AI Decision Autonomy**: Pluggable planner with cost/safety constraints
- **Modular Architecture**: Optional backends for LLM inference, vector indexing, and persistence

## Project Structure

```
RexOne/
├── CMakeLists.txt          # Build configuration with optional features
├── include/                # Public headers
│   ├── orchestrator.h      # Main orchestrator pipeline
│   ├── memory_manager.h    # Multi-tier memory system
│   ├── model_interface.h   # LLM inference interface
│   ├── tokenizer.h         # Token counting and encoding
│   ├── llama_backend.h     # Optional llama.cpp integration
│   └── vector_store.h      # Vector (embedding) storage interface
├── src/                    # Implementation
│   ├── main.cpp            # Interactive REPL demo
│   ├── orchestrator.cpp    # Orchestrator logic
│   ├── memory_manager.cpp  # Memory management implementation
│   ├── model_interface.cpp # Model inference (stubs + hooks)
│   ├── tokenizer.cpp       # Tokenizer implementation
│   ├── llama_backend.cpp   # Optional llama backend
│   ├── vector_store.cpp    # Vector store implementation
│   └── persistence.cpp     # Optional SQLite persistence
└── build/                  # Build output (generated)
```

## Building

### Prerequisites
- **CMake** 3.15+
- **C++17 compiler** (MSVC, GCC, or Clang)
- **Threads** library (usually built-in)

### Basic Build (Demo Mode)
```bash
cd C:\repo\RexOne
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The executable will be at `build/Release/rexone.exe`.

### Build with Optional Features
```bash
# Enable llama.cpp LLM backend
cmake -S . -B build -DUSE_LLAMA=ON

# Enable hnswlib vector indexing
cmake -S . -B build -DUSE_HNSW=ON

# Enable SQLite persistence
cmake -S . -B build -DUSE_SQLITE=ON

# All features
cmake -S . -B build -DUSE_LLAMA=ON -DUSE_HNSW=ON -DUSE_SQLITE=ON

# Then build:
cmake --build build --config Release
```

## Running the Demo REPL

```bash
.\build\Release\rexone.exe
```

Example session:
```
RexOne demo REPL. Type 'quit' to exit.
You> Comment fonctionnes-tu?
RexOne> Je comprends votre question. Voici ma perspective basée sur le contexte. (Streaming llama token par token)

You> Tu es rapide!
RexOne> Merci pour ce message. Je vais traiter votre demande avec attention. (Demo streaming actif)

You> quit
```

## Current Features

### Streaming LLM Backend
- **LlamaBackend class**: Modular C++ wrapper for llama.cpp integration
  - Token-by-token streaming via callback: `generate_streaming(prompt, maxTokens, callback)`
  - Blocking generation: `generate_blocking(prompt, maxTokens)`
  - Ready for real llama.cpp integration (currently using demo stubs)
  - Minimal simulated latency between tokens (5ms per token) for realistic UX
- **ModelInterface integration**: Transparently uses streaming backend
  - `generate()` — synchronous generation (blocks until completion)
  - `generate_streaming()` — asynchronous token streaming with callback
- **Fallback design**: If llama backend unavailable, falls back to simple demo generation

### Multi-tier Memory & RAG
- **Short-term**: Sliding window (2048 token budget) of recent conversation history
- **Long-term (RAG)**: VectorStore with semantic search using cosine similarity
  - All Q&A pairs are embedded and stored in long-term memory
  - When user queries, relevant past interactions are retrieved and included in prompt context
  - Enables the agent to "remember" and reference previous discussions
- **Question Detection**: Detects questions (contains `?`) and tailors responses
- **Deterministic Embeddings**: Uses hash-based embeddings (currently 8-dimensional) for semantic similarity
  - Ready to integrate real embedding models (BERT, OpenAI, etc.)

### Architecture: Orchestrator Pipeline
```
Input 
  → Append to Short-term History
  → Embed Query  
  → Retrieve Similar Facts from Long-term (RAG)
  → Compose Prompt (system instruction + history + retrieved facts + user input)
  → Model Generate Response
  → Embed & Store Q&A Pair to Long-term
  → Output
```

### Memory Layers
- **Short-term**: Sliding window (2048 token budget) of recent conversation
- **Mid-term**: Summarization compression for old context
- **Long-term**: Vector store with semantic search (RAG)

### Extensibility Points
1. **LlamaBackend**: Currently demo/stub; integrate real llama.cpp for production inference
   - API ready: call `init_global_llama("path/to/model.gguf")` to load a real GGUF model
   - Streaming tokens via callback already implemented
   - Just need to link actual llama.cpp library and call underlying inference
2. **ModelInterface**: Streaming generation interface is in place; integrate embedding models (BERT, etc.)
3. **Tokenizer**: Integrate HuggingFace tokenizers (BPE) for accurate token counting
4. **VectorStore**: Currently uses cosine similarity O(N); upgrade to hnswlib or FAISS for O(log N) search
5. **Embeddings**: Current hash-based embeddings are for demo; integrate real models (BERT, OpenAI, etc.)
6. **Persistence**: Enable SQLite (`-DUSE_SQLITE=ON`) for durable memory across sessions
7. **Planner**: Add symbolic constraints and utility functions for autonomous decision-making

## Next Steps (Implementation Roadmap)

1. **LLM Integration** (Phase 1 — ✅ IN PROGRESS)
   - ✅ Streaming LLM backend architecture (LlamaBackend class)
   - ✅ Token-by-token callback API ready
   - 🔄 Connect real llama.cpp for GGUF model inference
   - 🔄 Support quantized models (int8, int4) for embedded use

2. **Vector Indexing** (Phase 2 — ✅ FUNCTIONAL)
   - ✅ RAG with semantic search implemented
   - ✅ Cosine similarity working
   - 🔄 Wire hnswlib for O(log N) search on large corpora
   - 🔄 Implement chunking strategy for long documents
   - 🔄 Add importance scoring and TTL-based pruning

3. **Persistence** (Phase 3)
   - SQLite storage for memories across sessions
   - Async batch updates to avoid blocking inference
   - Export/import for cloud backup

4. **Autonomy & Safety** (Phase 4)
   - Planner with cost/risk evaluation
   - Human-in-the-loop for critical actions
   - Audit logs and explainability

## Building on Windows with VS Code

1. Open the folder in VS Code
2. Install **C/C++ Extension Pack** (if not already done)
3. **Command Palette** → `CMake: Select a Kit` → Choose MSVC or MinGW
4. **Command Palette** → `CMake: Configure`
5. **Command Palette** → `CMake: Build`
6. **Command Palette** → `CMake: Run without Debugging`

## Performance Considerations

- **Memory**: Aim for < 100MB resident footprint (with quantized models)
- **Latency**: Tokenization + embedding < 100ms, generation streaming at token/300ms on mobile CPU
- **Energy**: Dynamic model switching based on device state
- **Throughput**: Process multi-turn interactions asynchronously to avoid blocking

## License

This project is a demonstration/educational prototype.

## Notes

- **LLM Backend**: ✅ Streaming infrastructure in place (LlamaBackend class with token callbacks)
  - Demo mode uses simulated streaming tokens with 5ms latency
  - Ready for real llama.cpp integration: just call `init_global_llama("model.gguf")`
  - When USE_LLAMA is defined, code paths prepared for actual inference
- **Streaming Tokens**: ✅ Functional in demo; supports both blocking and callback-based generation
- **Embeddings**: Currently hash-based (8D); production should use real models (BERT, Sentence-Transformers)
- **Vector Search**: Cosine similarity O(N) working; production → hnswlib or FAISS for O(log N)
- **RAG Status**: ✅ Full pipeline working — Q&A pairs embedded, stored, and retrieved by semantic similarity
- **Tokenizer**: Whitespace-based stub; production should use BPE (SentencePiece, HuggingFace tokenizers)
- **Persistence**: SQLite integration available via `USE_SQLITE` flag
- **Encoding Note**: Character encoding (UTF-8) is preserved; some terminal output may show encoding artifacts

---

**RexOne** — Bringing autonomous AI into your pocket.

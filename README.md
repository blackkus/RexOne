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
│   ├── vector_store.h      # Vector (embedding) storage interface
│   └── planner.h           # Autonomous decision-making with safety constraints
├── src/                    # Implementation
│   ├── main.cpp            # Interactive REPL demo with planner commands
│   ├── orchestrator.cpp    # Orchestrator logic with planner integration
│   ├── memory_manager.cpp  # Memory management implementation
│   ├── model_interface.cpp # Model inference (stubs + hooks)
│   ├── tokenizer.cpp       # Tokenizer implementation
│   ├── llama_backend.cpp   # Optional llama backend
│   ├── vector_store.cpp    # Vector store implementation
│   ├── persistence.cpp     # Optional SQLite persistence
│   └── planner.cpp         # Autonomous planner implementation
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
Commands: autonomous [on|off] | decisions | plan | help

You> Comment fonctionnes-tu?
RexOne> Je comprends votre question. Voici ma perspective basée sur le contexte. (Streaming llama token par token)

You> autonomous on
RexOne> Autonomous mode enabled.

You> Tu es rapide!
RexOne> Merci pour ce message. Je vais traiter votre demande avec attention. (Demo streaming actif)

You> decisions
RexOne> Decision Audit Log:
  [2026-04-15 13:45:22] Action: cleanup_old_memories, Outcome: planned, Score: 0.700
  [2026-04-15 13:45:23] Action: export_backup, Outcome: planned, Score: 0.533

You> help
RexOne> Commands:
  quit/exit        - Exit the REPL
  autonomous on/off - Enable/disable autonomous mode
  decisions        - Show decision audit log
  plan             - Show planner info
  help/?           - Show this help

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

### Persistence & Durable Memory
- **Persistence class**: SQLite-backed memory storage (optional via `USE_SQLITE` flag)
  - Schema: memories table with `id`, `text`, `embedding`, `timestamp`, `importance`
  - Serialize embeddings as comma-separated floats for storage
  - Full CRUD: save, load, search, delete old memories
  - Graceful degradation: works without SQLite (stub mode) for demo portability
- **MemoryManager integration**:
  - `load_from_persistence()` — load all memories on startup
  - `save_to_persistence(text, embedding)` — async-ready method to persist Q&A pairs
  - Enables cross-session memory continuity
- **Cleanup**: Automatic removal of old memories (keep_since parameter) to prevent unbounded growth
- **Timestamps**: Each memory record includes Unix timestamp for temporal queries

### Autonomous Decision-Making & Safety (Planner)
- **Planner class**: Evaluates candidate actions and selects best option respecting safety constraints
  - **Utility function**: `utility = (benefit / cost) * (1 - risk * 0.5)` — balances reward vs resource usage while penalizing risky actions
  - **Safety constraints**: Symbolic rules that can be added/removed dynamically
    - `safety_max_risk(threshold)` — reject actions exceeding risk threshold
    - `safety_preserve_recent()` — prevent deletion of recent memories
    - `safety_require_approval()` — flag high-risk actions for human review
  - **Audit logging**: Every decision is timestamped and logged with reasoning for explainability
- **Integration with Orchestrator**:
  - `set_autonomous_mode(bool)` — enable periodic autonomous memory management decisions
  - `make_decision(candidates)` — evaluate action candidates and return best plan
  - `get_decisions_log()` — retrieve audit trail for transparency
- **REPL Commands** (demo):
  - `autonomous on/off` — toggle autonomous mode
  - `decisions` — display decision audit log
  - `plan` — show planner info
  - `help` — show all commands
- **Current Autonomy Candidates** (extensible):
  - `cleanup_old_memories` — remove data older than 30 days (cost 0.1, risk 0.1, benefit 0.7)
  - `compress_context` — summarize long-term memories (cost 0.3, risk 0.2, benefit 0.5)
  - `export_backup` — encrypted backup (cost 0.15, risk 0.0, benefit 0.8)
  - `idle` — continue normal operation (cost 0.0, risk 0.0, benefit 0.0)

### Architecture: Orchestrator Pipeline
```
Input 
  → Append to Short-term History
  → Embed Query  
  → Retrieve Similar Facts from Long-term (RAG)
  → Compose Prompt (system instruction + history + retrieved facts + user input)
  → Model Generate Response
  → Embed & Store Q&A Pair to Long-term & Persistence
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
7. **Planner**: Extend action candidates; add RL-based learning from outcomes; implement threat detection

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

3. **Persistence** (Phase 3 — ✅ IMPLEMENTED)
   - ✅ SQLite storage for memories across sessions
   - ✅ Serialize embeddings and store with timestamps
   - ✅ Load/save/cleanup APIs ready
   - 🔄 Async batch updates (thread pool) to avoid blocking inference
   - 🔄 Export/import for cloud backup

4. **Autonomy & Safety** (Phase 4 — ✅ IMPLEMENTED)
   - ✅ Planner class with cost/risk/benefit evaluation
   - ✅ Symbolic safety constraints with pre-defined rules
   - ✅ Audit logging for explainability and transparency
   - ✅ REPL commands for autonomous mode toggling and decision inspection
   - ✅ Memory management action candidates (cleanup, compress, backup, idle)
   - 🔄 RL-based action learning from outcomes
   - 🔄 Extended autonomy candidates (resource optimization, threat detection)

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
- **Persistence**: SQLite integration available via `USE_SQLITE` flag (enables cross-session memory durability)
- **Autonomy & Planner**: ✅ Decision-making framework in place
  - Evaluates memory management actions (cleanup, compress, backup) using utility function
  - Respects symbolic safety constraints (max risk, preserve recent memories)
  - Audits all decisions with timestamps and reasoning for explainability
  - Future: extend with RL-based learning and threat detection
- **Encoding Note**: Character encoding (UTF-8) is preserved; some terminal output may show encoding artifacts

---

**RexOne** — Bringing autonomous AI into your pocket.

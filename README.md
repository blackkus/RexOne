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
RexOne> RexOne (demo): Je vois une question. Voici une réponse courte.

You> Salut
RexOne> RexOne (demo): J'ai reçu votre message. (demo généré)

You> quit
```

The demo currently:
- Maintains a sliding window of recent conversation history
- Detects questions (contains `?`) and tailors responses
- Generates simple deterministic responses (stubs for real model integration)
- Uses basic embeddings for semantic similarity

## Architecture Overview

### Orchestrator Pipeline
```
Input → Normalize → Memory Append → Embed Query → Retrieve Facts
     → Compose Prompt → Model Generate → Post-process → Update Memory → Output
```

### Memory Layers
- **Short-term**: Sliding window (2048 token budget) of recent conversation
- **Mid-term**: Summarization compression for old context
- **Long-term**: Vector store with semantic search (RAG)

### Extensibility Points
1. **ModelInterface**: Replace with real LLM (llama.cpp, ONNX, FasterTransformer, etc.)
2. **Tokenizer**: Integrate HuggingFace tokenizers (BPE) for accurate token counting
3. **VectorStore**: Connect to hnswlib or FAISS for efficient nearest-neighbor search
4. **Persistence**: Enable SQLite for durable memory across sessions
5. **Planner**: Add symbolic constraints and utility functions for autonomous decision-making

## Next Steps (Implementation Roadmap)

1. **LLM Integration** (Phase 1)
   - Integrate llama.cpp for local model inference
   - Support quantized models (int8, int4) for embedded use
   - Add streaming token generation

2. **Vector Indexing** (Phase 2)
   - Wire hnswlib for fast semantic search
   - Implement chunking strategy for documents
   - Add importance scoring and TTL-based pruning

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

- Demo mode uses simple heuristic inference; real deployment requires actual LLM backend integration
- Tokenizer is whitespace-based stub; production should use BPE or SentencePiece
- Vector store uses naive cosine similarity; production recommendation: hnswlib or FAISS
- Persistence is stubbed; enable SQLite for durability

---

**RexOne** — Bringing autonomous AI into your pocket.

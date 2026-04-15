cmake_minimum_required(VERSION 3.15)
project(RexOne LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

option(USE_LLAMA "Enable llama.cpp backend" OFF)
option(USE_HNSW "Enable hnswlib vector store (header-only)" ON)
option(USE_SQLITE "Enable SQLite persistence" OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Fetch hnswlib (header-only, lightweight)
if(USE_HNSW)
    message(STATUS "Fetching hnswlib for vector similarity search...")
    FetchContent_Declare(
        hnswlib
        GIT_REPOSITORY https://github.com/nmslib/hnswlib.git
        GIT_TAG master
        SOURCE_SUBDIR cpp
    )
    FetchContent_GetProperties(hnswlib)
    if(NOT hnswlib_POPULATED)
        FetchContent_Populate(hnswlib)
    endif()
    include_directories(${hnswlib_SOURCE_DIR})
    add_definitions(-DUSE_HNSW)
endif()

# Base sources
set(SRCS
    src/main.cpp
    src/orchestrator.cpp
    src/memory_manager.cpp
    src/model_interface.cpp
    src/tokenizer.cpp
    src/llama_backend.cpp
    src/vector_store.cpp
)

# Optional: persistence
if(USE_SQLITE)
    list(APPEND SRCS src/persistence.cpp)
    add_definitions(-DUSE_SQLITE)
endif()

add_executable(rexone ${SRCS})

# Link threads for portability
find_package(Threads REQUIRED)
target_link_libraries(rexone PRIVATE Threads::Threads)

# Optional SQLite
if(USE_SQLITE)
    find_package(SQLite3)
    if(SQLite3_FOUND)
        target_include_directories(rexone PRIVATE ${SQLite3_INCLUDE_DIRS})
        target_link_libraries(rexone PRIVATE ${SQLite3_LIBRARIES})
    endif()
endif()

message(STATUS "Build options: USE_LLAMA=${USE_LLAMA}, USE_HNSW=${USE_HNSW}, USE_SQLITE=${USE_SQLITE}")

# Install target (optional)
install(TARGETS rexone RUNTIME DESTINATION bin)

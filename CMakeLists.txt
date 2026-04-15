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

if(USE_LLAMA)
    message(STATUS "Fetching llama.cpp for GGUF model support...")
    set(LLAMA_BUILD_COMMON OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_WEBUI OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        llama_cpp
        GIT_REPOSITORY https://github.com/ggerganov/llama.cpp.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(llama_cpp)
    add_definitions(-DUSE_LLAMA)
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
    src/persistence.cpp
    src/planner.cpp
)

# Optional: SQLite linking
if(USE_SQLITE)
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

if(USE_LLAMA)
    target_link_libraries(rexone PRIVATE llama)
    target_include_directories(rexone PRIVATE ${llama_cpp_SOURCE_DIR}/include)
    
    # Copy llama DLLs to executable directory on Windows
    if(WIN32)
        add_custom_command(TARGET rexone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:llama> $<TARGET_FILE_DIR:rexone>
            COMMENT "Copying llama DLL"
        )
    endif()
endif()

message(STATUS "Build options: USE_LLAMA=${USE_LLAMA}, USE_HNSW=${USE_HNSW}, USE_SQLITE=${USE_SQLITE}")

# Install target (optional)
install(TARGETS rexone RUNTIME DESTINATION bin)

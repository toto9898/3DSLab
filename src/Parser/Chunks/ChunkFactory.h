#pragma once

#include <memory>
#include <functional>
#include <unordered_map>
#include "Chunk.h"

namespace Debugger3DS {
    
    // Chunk factory for creating appropriate chunk types based on chunk ID
    class ChunkFactory {
    public:
        using ChunkCreator = std::function<std::shared_ptr<Chunk>()>;
        
        ChunkFactory() = default;
        
        // Register a chunk creator function for a specific chunk ID
        void RegisterChunk(uint16_t chunkId, ChunkCreator creator) {
            creators_[chunkId] = creator;
        }
        
        // Create a chunk based on its ID
        std::shared_ptr<Chunk> CreateChunk(const ChunkHeader& header, std::istream& stream) {
            auto startPos   = stream.tellg() - static_cast<std::streamoff>(6); // We already have the header read, so start position is 6 behind
            auto endPos     = startPos + static_cast<std::streamoff>(header.length);

            std::shared_ptr<Chunk> chunk;
            auto it = creators_.find(header.id);
            if (it != creators_.end()) {
                chunk = it->second();
            } else {
                chunk = std::make_shared<Chunk>(stream);
                stream.seekg(endPos);
            }
            
            chunk->header_ = header;
            chunk->dataEndPos_ = endPos;

            return chunk;
        }
        
        // Template helper for registering chunks
        template<typename ChunkType>
        void RegisterChunk(uint16_t chunkId, std::istream& stream) {
            RegisterChunk(chunkId, [&stream]() {
                return std::make_shared<ChunkType>(stream);
            });
        }
        
    private:
        std::unordered_map<uint16_t, ChunkCreator> creators_;
    };
        
} // namespace Debugger3DS
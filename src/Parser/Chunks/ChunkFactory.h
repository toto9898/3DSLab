#pragma once

#include <memory>
#include <functional>
#include <unordered_map>
#include "Chunk.h"

namespace Debugger3DS::Parser::Chunks {
    
    /// @brief Creates concrete `Chunk` subclasses from raw chunk headers.
    ///
    /// Each chunk type is registered via `RegisterChunk()` with a factory lambda.
    /// When a header is encountered during parsing, `CreateChunk()` looks up the
    /// registered creator; unknown IDs fall back to a base `Chunk` that skips
    /// the payload by seeking past it.
    class ChunkFactory {
    public:
        /// @brief Callable that allocates and returns a newly-constructed chunk.
        using ChunkCreator = std::function<std::shared_ptr<Chunk>()>;

        ChunkFactory() = default;

        /// @brief Register a factory lambda for @p chunkId.
        /// @param chunkId  The 16-bit chunk type identifier.
        /// @param creator  Callable that returns a new instance of the chunk.
        void RegisterChunk(uint16_t chunkId, ChunkCreator creator) {
            creators_[chunkId] = creator;
        }

        /// @brief Instantiate the correct `Chunk` subclass for @p header.
        ///
        /// The stream is assumed to be positioned immediately after the 6-byte
        /// header (i.e. at the first data byte).  If @p header.id is not
        /// registered, the stream is seeked past the chunk and a base `Chunk`
        /// is returned.
        ///
        /// @param header  Pre-read chunk header.
        /// @param stream  Binary input stream.
        /// @return        Ready-to-read chunk with `header_` and `dataEndPos_` set.
        std::shared_ptr<Chunk> CreateChunk(const ChunkHeader& header, std::istream& stream) {
            // We already read the 6-byte header, so the chunk started 6 bytes back.
            auto startPos = stream.tellg() - static_cast<std::streamoff>(6);
            auto endPos   = startPos + static_cast<std::streamoff>(header.length);

            std::shared_ptr<Chunk> chunk;
            auto it = creators_.find(header.id);
            if (it != creators_.end()) {
                chunk = it->second();
            } else {
                chunk = std::make_shared<Chunk>(stream);
                stream.seekg(endPos);
            }

            chunk->header_     = header;
            chunk->dataEndPos_ = endPos;
            return chunk;
        }

        /// @brief Convenience overload: register `ChunkType` (constructed with @p stream) for @p chunkId.
        template<typename ChunkType>
        void RegisterChunk(uint16_t chunkId, std::istream& stream) {
            RegisterChunk(chunkId, [&stream]() {
                return std::make_shared<ChunkType>(stream);
            });
        }

    private:
        std::unordered_map<uint16_t, ChunkCreator> creators_; ///< Map of chunk ID → factory lambda.
    };
        
} // namespace Debugger3DS::Parser::Chunks
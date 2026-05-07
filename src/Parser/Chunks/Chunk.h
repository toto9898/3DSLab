#pragma once

#include <memory>
#include <vector>
#include <istream>
#include <cstdint>
#include <string>
#include <functional>
#include <iostream>
#include <Eigen/Dense>
#include <ios>
#include "ChunkTypes.h"
#include "AnimationHeaders.h"
#include "Logger.h"

namespace Debugger3DS::Parser { class Importer; }

namespace Debugger3DS::Parser::Chunks {
    // (Importer is in parent namespace Debugger3DS::Parser — accessible as Importer below)
    
    /// @brief Fixed 6-byte header present at the start of every 3DS chunk.
    struct ChunkHeader {
        uint16_t id     = 0; ///< Chunk type identifier (see ChunkTypes.h).
        uint32_t length = 0; ///< Total chunk size in bytes, including this 6-byte header.
    };

    /// @cond INTERNAL
    template<typename T>
    constexpr bool is_trivially_copyable_v = std::is_trivially_copyable_v<T>;
    /// @endcond

    /// @brief Abstract base class for all 3DS file chunks.
    ///
    /// Each chunk owns a reference to the shared input stream.  The standard
    /// read sequence is:
    ///   1. `Read()` — orchestrates the full read: calls `ReadData()` then `ReadChildren()`.
    ///   2. `ReadData()` — parses the chunk's own payload into the `Importer`.
    ///   3. `ReadChildren()` — recursively reads nested child chunks.
    ///   4. `Process()` — optional post-processing hook called after the chunk tree is built.
    class Chunk {
    public:
        /// @brief Construct a chunk backed by @p stream.
        Chunk(std::istream& stream) : stream_(stream) {}
        virtual ~Chunk() = default;

        /// @brief Orchestrate a full read: `ReadData()` followed by `ReadChildren()`.
        /// @return @c false if any step fails.
        virtual bool Read(Importer& importer);

        /// @brief Parse this chunk's own payload and push data into @p importer.
        /// @return @c false on stream or parse error.
        virtual bool ReadData(Importer& importer);

        /// @brief Return @c true if @p childId is a valid child chunk type for this chunk.
        virtual bool ValidChild(uint16_t childId) const;

        /// @brief Recursively read all child chunks up to `dataEndPos_`.
        bool ReadChildren(Importer& importer);

        /// @brief Optional post-processing hook called after the whole chunk tree is read.
        virtual bool Process(Importer& importer);

        /// @brief Return a human-readable summary of this chunk's data (for logging).
        virtual std::string GetInfo() const;

        /// @brief Return the chunk's type name string (e.g. `"POS_TRACK_TAG"`).
        virtual std::string GetTypeName() const;

        /// @brief Return the chunk's 16-bit type identifier.
        uint16_t GetId() const;

        /// @brief Read the next chunk header from @p stream and dispatch to the factory.
        /// @return The constructed (and possibly specialised) chunk, or a base `Chunk` on unknown IDs.
        static std::shared_ptr<Chunk> CreateChunk(std::istream& stream, Importer& importer);

    protected:
        /// @brief Read a null-terminated ASCII string from the stream.
        bool Read(std::string& value);

        /// @brief Read a trivially-copyable value directly from an arbitrary @p stream.
        template<typename T>
        static bool Read(T& value, std::istream& stream) {
            stream.read(reinterpret_cast<char*>(&value), sizeof(value));
            return stream.good();
        }

        /// @brief Read a trivially-copyable value from the chunk's own stream, with bounds check.
        template<typename T>
        bool Read(T& value) {
            return Chunk::Read(value, stream_) && stream_.tellg() <= dataEndPos_;
        }

    protected:
        ChunkHeader header_;              ///< The chunk's 6-byte header (id + length).
        uint16_t parentId = 0;            ///< Type ID of the parent chunk (set by factory).

        std::istream&  stream_;           ///< Shared binary input stream.
        std::streampos dataEndPos_ = 0;   ///< Stream position one byte past this chunk's data.

        friend class Debugger3DS::Parser::Importer;
        friend class ChunkFactory;
    };
    
} // namespace Debugger3DS::Parser::Chunks
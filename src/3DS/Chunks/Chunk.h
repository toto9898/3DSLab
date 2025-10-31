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
#include "../ChunkTypes.h"
#include "../Animation/AnimationHeaders.h"
#include "../../Logger.h"

namespace Debugger3DS {
    // Forward declarations
    class Importer;
    
    // Base chunk header structure
    struct ChunkHeader {
        uint16_t id     = 0;  // Chunk type identifier
        uint32_t length = 0;  // Total chunk length including header
    };
    // Type trait for trivially copyable types (C++20 concept alternative)
    template<typename T>
    constexpr bool is_trivially_copyable_v = std::is_trivially_copyable_v<T>;
    
    // Abstract base class for all chunks
    class Chunk {
    public:
        Chunk(std::istream& stream) : stream_(stream) {}
        virtual ~Chunk() = default;
        
        // Virtual function to read chunk data from stream - now has default implementation
        virtual bool Read(Importer& importer);
        virtual bool ReadData(Importer& importer);
        virtual bool ValidChild(uint16_t childId) const;
        
        // Read child chunks from stream
        bool ReadChildren(Importer& importer);
        
        // Virtual function to process chunk after reading (for post-processing)
        virtual bool Process(Importer& importer);
        
        // Virtual function to get chunk information for debugging/display
        virtual std::string GetInfo() const;
        
        // Get chunk type name for display
        virtual std::string GetTypeName() const;
        
        // Getters
        uint16_t GetId() const;
        
        // Create and read a chunk from stream (factory method)
        static std::shared_ptr<Chunk> CreateChunk(std::istream& stream, Importer& importer);
        
    // Read support for primitive types
    protected:
        bool Read(std::string& value);

        template<typename T>
        static bool Read(T& value, std::istream& stream) {
            stream.read(reinterpret_cast<char*>(&value), sizeof(value));
            return stream.good();
        }
        
        template<typename T>
        bool Read(T& value) {
            return Chunk::Read(value, stream_) && stream_.tellg() <= dataEndPos_;
        }

    protected:
        ChunkHeader header_;
        uint16_t parentId = 0;
        std::vector<std::shared_ptr<Chunk>> children_;

        std::istream& stream_;
        std::streampos dataEndPos_ = 0;

        friend class Importer;
        friend class ChunkFactory;
    };
    
} // namespace Debugger3DS
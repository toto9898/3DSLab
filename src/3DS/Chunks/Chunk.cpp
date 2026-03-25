#include "Chunk.h"
#include "../Importer.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace Debugger3DS {
    
    // Process method implementation
    bool Chunk::Process(Importer& importer) {
        return true;
    }
    
    // Read complete chunk including data and children
    bool Chunk::Read(Importer& importer) {
        std::streampos chunkStart = stream_.tellg();

        if (!ReadData(importer)) {
            return false;
        }

        if (!ReadChildren(importer)) {
            return false;
        }
        
        return Process(importer);
    }

    bool Chunk::ReadData(Importer& importer)
    {
        return true;
    }

    bool Chunk::ValidChild(uint16_t childId) const
    {
        return true;
    }

    // Read child chunks from stream
    bool Chunk::ReadChildren(Importer& importer) {
        size_t bytesRead = 0;

        while (stream_.good() && stream_.tellg() < dataEndPos_) {
            std::streampos beforeRead = stream_.tellg();
            
            std::shared_ptr<Chunk> childChunk = CreateChunk(stream_, importer);
            if (!childChunk) {
                logging::log << "Failed to create child chunk in " << GetTypeName()
                             << " at stream position " << beforeRead << std::endl;
                break;
            }

            childChunk->parentId = GetId();

            if (!childChunk->Read(importer)) {
                logging::log << "Failed to read child chunk " << childChunk->GetTypeName()
                             << " (0x" << std::hex << childChunk->GetId() << std::dec
                             << ") inside " << GetTypeName()
                             << " at stream position " << beforeRead << std::endl;
                return false;
            }

            std::streampos afterRead = stream_.tellg();
            size_t chunkSize = static_cast<size_t>(afterRead - beforeRead);

            children_.push_back(std::move(childChunk));
            bytesRead += chunkSize;
        }

        if (stream_.tellg() != dataEndPos_) {
            logging::log << "Chunk " << GetTypeName() << " (0x" << std::hex << GetId() << std::dec
                         << "): expected end at " << dataEndPos_
                         << " but stream at " << stream_.tellg() << std::endl;
        }

        return stream_.tellg() == dataEndPos_;
    }

    std::shared_ptr<Chunk> Chunk::CreateChunk(std::istream& stream, Importer& importer) {
        std::streampos  chunkStart = stream.tellg();
        ChunkHeader     header_;
        if (!Read(header_.id, stream) ||
            !Read(header_.length, stream)) {
            return nullptr;
        }

        std::shared_ptr<Chunk> chunk = importer.GetChunkFactory().CreateChunk(header_, stream);
        
        return chunk;
    }

    bool Chunk::Read(std::string &value)
    {
        value.clear();
        char c;
        while (stream_.read(&c, 1) && c != '\0') {
            value += c;
        }
        return stream_.good() && stream_.tellg() <= dataEndPos_;
    }

    uint16_t Chunk::GetId() const {
        return header_.id;
    }
    
    // Default implementation returns basic chunk info
    std::string Chunk::GetInfo() const {
        std::ostringstream oss;
        oss << "ID: 0x" << std::hex << std::setw(4) << std::setfill('0') << header_.id
            << " Length: " << std::dec << header_.length << " bytes";
        return oss.str();
    }
    
    // Default implementation returns hex ID
    std::string Chunk::GetTypeName() const {
        std::ostringstream oss;
        oss << "Chunk_0x" << std::hex << std::setw(4) << std::setfill('0') << header_.id;
        return oss.str();
    }
    
} // namespace Debugger3DS

#include "Importer.h"
#include "Chunks/Chunk.h"
#include "Chunks/FaceArrayChunk.h"
#include "Chunks/ChunkRegistration.h"
#include <iostream>
#include <fstream>
#include <streambuf>

namespace Debugger3DS {
    
    // In-memory stream buffer — wraps a contiguous char buffer as a std::istream source.
    // Eliminates per-read disk I/O by loading the entire file into RAM first.
    class MemoryStreamBuf : public std::streambuf {
    public:
        MemoryStreamBuf(char* data, size_t size) {
            setg(data, data, data + size);
        }
    protected:
        pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode) override {
            char* ptr = (dir == std::ios_base::beg) ? eback() + off :
                        (dir == std::ios_base::cur) ? gptr() + off :
                                                      egptr() + off;
            if (ptr < eback() || ptr > egptr()) return pos_type(off_type(-1));
            setg(eback(), ptr, egptr());
            return ptr - eback();
        }
        pos_type seekpos(pos_type pos, std::ios_base::openmode mode) override {
            return seekoff(off_type(pos), std::ios_base::beg, mode);
        }
    };

    bool Importer::Import3DS(const std::string& filename) {
        
        // Open file and read entire contents into memory
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "[Parser] Could not open file: " << filename << std::endl;
            return false;
        }
        
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<char> fileBuffer(static_cast<size_t>(fileSize));
        if (!file.read(fileBuffer.data(), fileSize)) {
            std::cerr << "[Parser] Failed to read file into memory: " << filename << std::endl;
            return false;
        }
        file.close();
        
        // Wrap the in-memory buffer as a std::istream
        MemoryStreamBuf membuf(fileBuffer.data(), fileBuffer.size());
        std::istream memstream(&membuf);
        
        // Register all chunk types
        RegisterAllChunks(chunkFactory_, memstream);

        logging::log << "Loading 3DS file: " << filename << " (" << fileSize << " bytes)" << std::endl;
        
        // Read the main chunk using the new Chunk method
        std::shared_ptr<Chunk> mainChunk = Chunk::CreateChunk(memstream, *this);
        if (!mainChunk) {
            std::cerr << "[Parser] Failed to create main chunk in: " << filename << std::endl;
            return false;
        } else if (mainChunk->Read(*this) == false) {
            std::cerr << "[Parser] Failed to read 3DS data in: " << filename << std::endl;
            return false;
        }
        
        // Verify it's a valid 3DS file
        if (mainChunk->GetId() != ChunkType::M3DMAGIC) {
            std::cerr << "[Parser] Not a valid 3DS file (missing M3DMAGIC): " << filename << std::endl;
            return false;
        }
        
        scene_.BuildObjectNodeHierarchy();
        scene_.BuildAnimationNodeAssociations();

        logging::log << "Successfully loaded 3DS file" << std::endl;
        scene_.PrintInfo();
        
        return true;
    }
    
    void Importer::SetKeyframeHeader(uint16_t revision, const std::string& filename, uint32_t animLength) {
        scene_.kfRevision = revision;
        scene_.kfFilename = filename;
        scene_.animationLength = animLength;
    }
    
    void Importer::SetKeyframeSegment(uint32_t start, uint32_t end) {
        scene_.segmentStart = start;
        scene_.segmentEnd = end;
    }
    
    void Importer::SetKeyframeCurrentTime(uint32_t frame) {
        scene_.currentFrame = frame;
    }
    
} // namespace Debugger3DS
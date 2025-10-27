#pragma once
#include <istream>


namespace Debugger3DS {
    
    // Base chunk header structure
    struct ChunkHeader {
        uint16_t id     = 0;      // Chunk type identifier
        uint32_t length = 0;  // Total chunk length including header
    };

    // File I/O utility functions
    class FileIO {
    public:
        FileIO(std::istream& stream) : stream_(stream) {}
       
        // Read chunk header from stream
        static bool ReadHeader(std::istream& stream, ChunkHeader& header);

        // Data type reading functions based on 3DS specification
        
        // Read unsigned 8-bit integer
        bool ReadUByte(uint8_t& value);
        
        // Read unsigned 16-bit integer (little-endian)
        bool ReadUShort(uint16_t& value);
        
        // Read signed 16-bit integer (little-endian)
        bool ReadShort(int16_t& value);
        
        // Read unsigned 32-bit integer (little-endian)
        bool ReadUInt(uint32_t& value);
        
        // Read signed 32-bit integer (little-endian)
        bool ReadLong(int32_t& value);
        
        // Read unsigned 32-bit integer (same as UInt for compatibility)
        bool ReadULong(uint32_t& value);
        
        // Read 32-bit IEEE floating point value (little-endian)
        bool ReadFloat(float& value);
        
        // Read null-terminated string (variable length)
        bool ReadString(std::string& value);
        
        // Read percentage as integer (0-100)
        bool ReadIntPercentage(int16_t& percentage);
        
        // Read percentage as float (0.0-1.0)
        bool ReadFloatPercentage(float& percentage);        

    protected:
        std::istream& stream_;
        std::streampos dataEndPos_ = 0;
    };
    
} // namespace Debugger3DS
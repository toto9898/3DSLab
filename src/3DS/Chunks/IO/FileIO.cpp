#include "FileIO.h"
#include "../Importer.h"

namespace Debugger3DS {

    struct ChunkHeader;

     // Implement the static data reading functions in the cpp file
    bool FileIO::ReadHeader(std::istream& stream, ChunkHeader& header) {
        std::streampos chunkStart = stream.tellg();
        stream.read(reinterpret_cast<char*>(&header.id), sizeof(header.id));
        stream.read(reinterpret_cast<char*>(&header.length), sizeof(header.length));
        return stream.good();
    }
    
    bool FileIO::ReadUByte(uint8_t& value) {
        stream_.read(reinterpret_cast<char*>(&value), sizeof(value));
        return stream_.good() && stream_.tellg() <= dataEndPos_;
    }
    
    bool FileIO::ReadUShort(uint16_t& value) {
        stream_.read(reinterpret_cast<char*>(&value), sizeof(value));
        return stream_.good() && stream_.tellg() <= dataEndPos_;
    }
    
    bool FileIO::ReadShort(int16_t& value) {
        stream_.read(reinterpret_cast<char*>(&value), sizeof(value));
        return stream_.good() && stream_.tellg() <= dataEndPos_;
    }
    
    bool FileIO::ReadUInt(uint32_t& value) {
        stream_.read(reinterpret_cast<char*>(&value), sizeof(value));
        return stream_.good() && stream_.tellg() <= dataEndPos_;
    }
    
    bool FileIO::ReadLong(int32_t& value) {
        stream_.read(reinterpret_cast<char*>(&value), sizeof(value));
        return stream_.good() && stream_.tellg() <= dataEndPos_;
    }
    
    bool FileIO::ReadULong(uint32_t& value) {
        return ReadUInt(value) && stream_.tellg() <= dataEndPos_;
    }
    
    bool FileIO::ReadFloat(float& value) {
        stream_.read(reinterpret_cast<char*>(&value), sizeof(value));
        return stream_.good() && stream_.tellg() <= dataEndPos_;
    }
    
    bool FileIO::ReadString(std::string& value) {
        value.clear();
        char c;
        while (stream_.read(&c, 1) && c != '\0') {
            value += c;
        }
        return stream_.good() && stream_.tellg() <= dataEndPos_;
    }
    
    bool FileIO::ReadIntPercentage(int16_t& percentage) {
        return ReadShort(percentage) && percentage >= 0 && percentage <= 100;
    }
    
    bool FileIO::ReadFloatPercentage(float& percentage) {
        return ReadFloat(percentage) && percentage >= 0.0f && percentage <= 1.0f;
    }

}
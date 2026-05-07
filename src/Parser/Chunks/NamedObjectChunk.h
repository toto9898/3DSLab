#pragma once

#include "Chunk.h"
#include <string>
#include <memory>

namespace Debugger3DS {
    
    // Forward declaration
    class NamedObject;
    
    // Named object chunk
    class NamedObjectChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        bool Process(Importer& importer) override;
        const std::string& GetObjectName() const;
        
    private:
        std::string name_;
        std::shared_ptr<NamedObject> targetObject_ = nullptr;
    };
    
} // namespace Debugger3DS
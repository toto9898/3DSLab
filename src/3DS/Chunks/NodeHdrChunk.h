#pragma once

#include "Chunk.h"
#include <Eigen/Dense>
#include <memory>

namespace Debugger3DS {

    // Forward declaration
    class NamedObject;

    // Node header chunk
    class NodeHdrChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        
        const std::string& GetName() const;
        uint16_t GetFlags1() const;
        uint16_t GetFlags2() const;
        uint16_t GetParentIndex() const;
        
    private:
        std::string name_;
        uint16_t flags1_ = 0;
        uint16_t flags2_ = 0;
        uint16_t parentId_ = 0;
        std::shared_ptr<NamedObject> targetObject_ = nullptr;  // Reference to scene object
    };
    
} // namespace Debugger3DS
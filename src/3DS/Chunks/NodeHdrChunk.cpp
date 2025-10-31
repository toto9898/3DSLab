#include "NodeHdrChunk.h"
#include "../Importer.h"
#include <iostream>

namespace Debugger3DS {

    bool NodeHdrChunk::ReadData(Importer& importer) {
        // Read node header data
        if (!Read(name_)) {
            return false;
        }
        if (!Read(flags1_) || !Read(flags2_) || !Read(parentIndex_)) {
            return false;
        }

        if (auto currentObjectNode = importer.GetCurrentObjectNode()) {
            currentObjectNode->associatedMeshName = name_;
            currentObjectNode->parentIndex = parentIndex_;
            currentObjectNode->nodeFlags = flags1_;
        }

        logging::log << "Node Header: Name=" << name_ << " Flags1=" << flags1_ << " Flags2=" << flags2_ << " ParentIndex=" << parentIndex_ << std::endl;
        return true;
    }

    const std::string& NodeHdrChunk::GetName() const {
        return name_;
    }

    uint16_t NodeHdrChunk::GetFlags1() const {
        return flags1_;
    }

    uint16_t NodeHdrChunk::GetFlags2() const {
        return flags2_;
    }

    uint16_t NodeHdrChunk::GetParentIndex() const {
        return parentIndex_;
    }

} // namespace Debugger3DS

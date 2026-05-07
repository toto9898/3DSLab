#include "NodeHdrChunk.h"
#include "Importer.h"
#include <iostream>

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;

    bool NodeHdrChunk::ReadData(Importer& importer) {
        // Read node header data
        if (!Read(name_)) {
            return false;
        }
        if (!Read(flags1_) || !Read(flags2_) || !Read(parentId_)) {
            return false;
        }

        if (auto currentObjectNode = importer.GetCurrentObjectNode()) {
            currentObjectNode->associatedMeshName = name_;
            currentObjectNode->parentId = parentId_;
            currentObjectNode->nodeFlags = flags1_;
        } else if (auto camNode = importer.GetCurrentCameraNode()) {
            camNode->cameraName = name_;
            camNode->parentId   = parentId_;
        } else if (auto camTarget = importer.GetCurrentCameraTargetNode()) {
            camTarget->name = name_;
        } else if (auto lightNode = importer.GetCurrentLightNode()) {
            lightNode->lightName = name_;
            lightNode->parentId  = parentId_;
        } else if (auto lightTarget = importer.GetCurrentLightTargetNode()) {
            lightTarget->name = name_;
        }

        logging::log << "Node Header: Name=" << name_ << " Flags1=" << flags1_ << " Flags2=" << flags2_ << " ParentIndex=" << parentId_ << std::endl;
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
        return parentId_;
    }

} // namespace Debugger3DS::Parser::Chunks

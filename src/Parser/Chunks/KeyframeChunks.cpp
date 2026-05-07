#include "KeyframeChunks.h"
#include "Importer.h"
#include <iostream>

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;

    bool KfHdrChunk::ReadData(Importer& importer) {
        // Read revision
        if (!Read(revision_)) {
            return false;
        }
        
        // Read filename (null-terminated string)
        if (!Read(filename_)) {
            return false;
        }
        
        // Read animation length
        if (!Read(animLength_)) {
            return false;
        }
        
        importer.SetKeyframeHeader(revision_, filename_, animLength_);
        
        logging::log << "Keyframe Header - Revision: " << revision_ 
                  << ", File: " << filename_ 
                  << ", Animation Length: " << animLength_ << " frames" << std::endl;
        
        return true;
    }
    
    std::string KfHdrChunk::GetInfo() const {
        return "Keyframe Header - Rev: " + std::to_string(revision_) + 
               ", File: " + filename_ + 
               ", Length: " + std::to_string(animLength_) + " frames";
    }

    bool KfSegChunk::ReadData(Importer& importer) {
        // Read start frame
        if (!Read(start_)) {
            return false;
        }
        
        // Read end frame
        if (!Read(end_)) {
            return false;
        }
        
        importer.SetKeyframeSegment(start_, end_);
        
        logging::log << "Keyframe Segment - Start: " << start_ 
                  << ", End: " << end_ << std::endl;
        
        return true;
    }
    
    std::string KfSegChunk::GetInfo() const {
        return "Keyframe Segment - Start: " + std::to_string(start_) + 
               ", End: " + std::to_string(end_);
    }

    bool KfCurTimeChunk::ReadData(Importer& importer) {
        // Read current frame
        if (!Read(currentFrame_)) {
            return false;
        }
        
        importer.SetKeyframeCurrentTime(currentFrame_);
        
        logging::log << "Current Time Frame: " << currentFrame_ << std::endl;
        
        return true;
    }
    
    std::string KfCurTimeChunk::GetInfo() const {
        return "Current Time Frame: " + std::to_string(currentFrame_);
    }

} // namespace Debugger3DS::Parser::Chunks
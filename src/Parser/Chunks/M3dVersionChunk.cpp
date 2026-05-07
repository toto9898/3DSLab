#include "M3dVersionChunk.h"
#include "Importer.h"
#include <iostream>
#include <sstream>

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;
    
    bool M3dVersionChunk::ReadData(Importer& importer) {
        
        if (!Read(version_)) {
            return false;
        }
        logging::log << "3DS File Version: " << version_ << std::endl;
        importer.SetVersion(version_);
        return true;
    }

    uint32_t M3dVersionChunk::GetVersion() const
    {
        return version_;
    }

    std::string M3dVersionChunk::GetInfo() const {
        std::ostringstream oss;
        oss << "Version: " << version_;
        return oss.str();
    }
    
    std::string M3dVersionChunk::GetTypeName() const {
        return "M3D_VERSION";
    }
    
} // namespace Debugger3DS::Parser::Chunks
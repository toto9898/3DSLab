#include "MasterScaleChunk.h"
#include "Importer.h"
#include <iostream>

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;
    
    bool MasterScaleChunk::ReadData(Importer& importer) {        
        if (!Read(scale_)) {
            return false;
        }
        logging::log << "Master Scale: " << scale_ << std::endl;
        importer.SetMasterScale(scale_);
        return true;
    }

    float MasterScaleChunk::GetScale() const
    {
        return scale_;
    }

} // namespace Debugger3DS::Parser::Chunks
#include "NamedObjectChunk.h"
#include "../Importer.h"
#include <iostream>

namespace Debugger3DS {
    
    bool NamedObjectChunk::ReadData(Importer& importer) {        
        if (!Read(name_)) {
            return false;
        }
        importer.SetCurrentObjectName(name_);

        logging::log << "Named Object: " << name_ << std::endl;
        return true;
    }

    const std::string &NamedObjectChunk::GetObjectName() const
    {
        return name_;
    }

} // namespace Debugger3DS
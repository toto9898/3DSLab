#include "NamedObjectChunk.h"
#include "Importer.h"
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

    bool NamedObjectChunk::Process(Importer& importer) {
        // Clear per-object state so it doesn't leak into subsequent NAMED_OBJECTs
        // or into the KFDATA section
        importer.SetCurrentMesh(nullptr);
        importer.SetCurrentLight(nullptr);
        importer.SetCurrentCamera(nullptr);
        return true;
    }

    const std::string &NamedObjectChunk::GetObjectName() const
    {
        return name_;
    }

} // namespace Debugger3DS
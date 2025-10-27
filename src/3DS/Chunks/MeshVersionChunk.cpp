#include "MeshVersionChunk.h"
#include "../Importer.h"
#include <iostream>

namespace Debugger3DS {
    
    bool MeshVersionChunk::ReadData(Importer& importer) {
        if (!ReadULong(version_)) {
            return false;
        }
        logging::log << "Mesh Version: " << version_ << std::endl;
        importer.SetMeshVersion(version_);
        return true;
    }

    uint32_t MeshVersionChunk::GetVersion() const
    {
        return version_;
    }

} // namespace Debugger3DS
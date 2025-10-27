#include "MatNameChunk.h"
#include "../Importer.h"
#include "../SceneObjects/Material.h"
#include <iostream>

namespace Debugger3DS {
    
    bool MatNameChunk::ReadData(Importer& importer) {
        if (!ReadString(name_)) {
            return false;
        }
        
        // Create new material and set as current
        targetMaterial_ = std::make_shared<Material>(name_);
        importer.GetScene().materials.push_back(targetMaterial_);
        importer.SetCurrentMaterial(targetMaterial_);
        importer.SetCurrentMaterialName(name_);
        
        logging::log << "Material Name: " << name_ << std::endl;
        return true;
    }

    const std::string &MatNameChunk::GetMaterialName() const
    {
        return name_;
    }

} // namespace Debugger3DS
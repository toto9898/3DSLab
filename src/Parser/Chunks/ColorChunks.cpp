#include "ColorChunks.h"
#include "Importer.h"
#include "Material.h"

namespace Debugger3DS {
    
    template<typename T>
    bool ColorChunk<T>::ReadData(Importer& importer) {
        T r, g, b;
        if (!Read(r) || !Read(g) || !Read(b)) {
            return false;
        }

        Eigen::Vector3f color;
        if constexpr (std::is_same_v<T, uint8_t>) {
            color = Eigen::Vector3f(r / 255.0f, g / 255.0f, b / 255.0f);
        } else {
            color = Eigen::Vector3f(r, g, b);
        }

        // When nested inside a light chunk, set the light color
        if (parentId == ChunkType::N_DIRECT_LIGHT) {
            if (auto light = importer.GetCurrentLight())
                light->color = color;
            return true;
        }

        // When nested inside AMBIENT_LIGHT, set scene ambient color
        if (parentId == ChunkType::AMBIENT_LIGHT) {
            importer.GetScene().ambientLight = color;
            return true;
        }

        // Otherwise route to the correct material property based on parent chunk
        auto material = importer.GetCurrentMaterial();
        if (!material) {
            return false;
        }

        const char* propName = "diffuse";
        switch (parentId) {
            case ChunkType::MAT_AMBIENT:
                material->ambient = color;
                propName = "ambient";
                break;
            case ChunkType::MAT_SPECULAR:
                material->specular = color;
                propName = "specular";
                break;
            default:
                material->diffuse = color;
                break;
        }
        
        if constexpr (std::is_same_v<T, uint8_t>) {
            logging::log << "Color " << propName << " (24-bit): R=" << (int)r << " G=" << (int)g << " B=" << (int)b << std::endl;
        } else {
            logging::log << "Color " << propName << " (float): R=" << r << " G=" << g << " B=" << b << std::endl;
        }
        
        return true;
    }
    
    // Explicit instantiations
    template class ColorChunk<uint8_t>;
    template class ColorChunk<float>;

} // namespace Debugger3DS

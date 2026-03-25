#include "ColorChunks.h"
#include "../Importer.h"
#include "../SceneObjects/Material.h"

namespace Debugger3DS {
    
    template<typename T>
    bool ColorChunk<T>::ReadData(Importer& importer) {
        auto material = importer.GetCurrentMaterial();
        if (!material) {
            return false;
        }
        
        T r, g, b;
        if (!Read(r) || !Read(g) || !Read(b)) {
            return false;
        }
        
        if constexpr (std::is_same_v<T, uint8_t>) {
            material->diffuse = Eigen::Vector3f(r / 255.0f, g / 255.0f, b / 255.0f);
            logging::log << "Color (24-bit): R=" << (int)r << " G=" << (int)g << " B=" << (int)b << std::endl;
        } else {
            material->diffuse = Eigen::Vector3f(r, g, b);
            logging::log << "Color (float): R=" << r << " G=" << g << " B=" << b << std::endl;
        }
        
        return true;
    }
    
    // Explicit instantiations
    template class ColorChunk<uint8_t>;
    template class ColorChunk<float>;

} // namespace Debugger3DS

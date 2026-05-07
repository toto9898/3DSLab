#include "LightChunks.h"
#include "Importer.h"
#include "Light.h"
#include "Scene.h"
#include <sstream>

namespace Debugger3DS {

    bool NDirectLightChunk::ReadData(Importer& importer) {
        Eigen::Vector3f position;
        if (!Read(position.x()) || !Read(position.y()) || !Read(position.z()))
            return false;

        auto light = std::make_shared<Light>(importer.GetCurrentObjectName());
        light->position = position;
        importer.GetScene().lights.push_back(light);
        importer.SetCurrentLight(light);

        logging::log << "N_DIRECT_LIGHT: \"" << light->name << "\" at ("
                     << position.x() << ", " << position.y() << ", " << position.z() << ")" << std::endl;
        return true;
    }

    bool DLSpotlightChunk::ReadData(Importer& importer) {
        auto light = importer.GetCurrentLight();
        if (!light)
            return false;

        if (!Read(light->target.x()) || !Read(light->target.y()) || !Read(light->target.z()))
            return false;
        if (!Read(light->hotspotAngle) || !Read(light->falloffAngle))
            return false;

        light->isSpotlight = true;

        logging::log << "DL_SPOTLIGHT: hotspot=" << light->hotspotAngle
                     << " falloff=" << light->falloffAngle << std::endl;
        return true;
    }

    bool DLOffChunk::ReadData(Importer& importer) {
        auto light = importer.GetCurrentLight();
        if (!light)
            return false;

        light->isOn = false;
        logging::log << "DL_OFF: light \"" << light->name << "\" is off" << std::endl;
        return true;
    }

    bool DLInnerRangeChunk::ReadData(Importer& importer) {
        auto light = importer.GetCurrentLight();
        if (!light) return false;
        if (!Read(light->innerRange)) return false;
        logging::log << "DL_INNER_RANGE: " << light->innerRange << std::endl;
        return true;
    }

    bool DLOuterRangeChunk::ReadData(Importer& importer) {
        auto light = importer.GetCurrentLight();
        if (!light) return false;
        if (!Read(light->outerRange)) return false;
        logging::log << "DL_OUTER_RANGE: " << light->outerRange << std::endl;
        return true;
    }

    bool DLMultiplierChunk::ReadData(Importer& importer) {
        auto light = importer.GetCurrentLight();
        if (!light) return false;
        if (!Read(light->multiplier)) return false;
        logging::log << "DL_MULTIPLIER: " << light->multiplier << std::endl;
        return true;
    }

} // namespace Debugger3DS

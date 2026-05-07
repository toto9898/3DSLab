#include "CameraChunks.h"
#include "Importer.h"
#include "Camera.h"
#include "Scene.h"

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;

    bool NCameraChunk::ReadData(Importer& importer) {
        Eigen::Vector3f position, target;
        float bank, focalLength;

        if (!Read(position.x()) || !Read(position.y()) || !Read(position.z()))
            return false;
        if (!Read(target.x()) || !Read(target.y()) || !Read(target.z()))
            return false;
        if (!Read(bank) || !Read(focalLength))
            return false;

        auto camera = std::make_shared<Camera>(importer.GetCurrentObjectName());
        camera->position  = position;
        camera->target    = target;
        camera->bankAngle = bank;
        camera->focalLength = focalLength;

        importer.GetScene().cameras.push_back(camera);
        importer.SetCurrentCamera(camera);

        logging::log << "N_CAMERA: \"" << camera->name << "\" focal=" << focalLength << "mm" << std::endl;
        return true;
    }

    bool CamRangesChunk::ReadData(Importer& importer) {
        auto camera = importer.GetCurrentCamera();
        if (!camera) return false;
        if (!Read(camera->nearRange) || !Read(camera->farRange)) return false;
        logging::log << "CAM_RANGES: near=" << camera->nearRange << " far=" << camera->farRange << std::endl;
        return true;
    }

} // namespace Debugger3DS::Parser::Chunks

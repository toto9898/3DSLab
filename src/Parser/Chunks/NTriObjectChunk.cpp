#include "NTriObjectChunk.h"
#include "Importer.h"
#include "Scene.h"
#include <iostream>

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;
    
    bool NTriObjectChunk::ReadData(Importer& importer) {

        mesh_ = std::make_shared<Mesh>(importer.GetCurrentObjectName());
        importer.SetCurrentMesh(mesh_);
        importer.GetScene().meshes.push_back(mesh_);

        logging::log << "Processing mesh object chunk" << std::endl;
        return true;
    }
    
} // namespace Debugger3DS::Parser::Chunks
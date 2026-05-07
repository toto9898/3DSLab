#include "PercentageChunk.h"
#include "Importer.h"
#include "Material.h"
#include <iostream>

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;
    
    bool IntPercentageChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // Read percentage value
        int16_t percentage;
        if (!Read(percentage)) {
            return false;
        }
        
        // Determine what property to set based on parent chunk
        SetMaterialProperty(percentage, parentId);
        
        return true;
    }
    
    void IntPercentageChunk::SetMaterialProperty(int16_t percentage, uint16_t parentChunkId) {
        float normalizedValue = percentage / 100.0f;
        
        switch (parentChunkId) {
            case ChunkType::MAT_SHININESS:
                targetMaterial_->shininess = normalizedValue * 128.0f;
                logging::log << "Material Shininess: " << percentage << "% (exponent " << targetMaterial_->shininess << ")" << std::endl;
                break;
                
            case ChunkType::MAT_SHIN2PCT:
                targetMaterial_->shininessPercent = normalizedValue;
                logging::log << "Material Specular Strength: " << percentage << "%" << std::endl;
                break;
                
            case ChunkType::MAT_TRANSPARENCY:
                targetMaterial_->transparency = normalizedValue;
                logging::log << "Material Transparency: " << percentage << "%" << std::endl;
                break;
                
            case ChunkType::MAT_XPFALL:
                targetMaterial_->transparencyFalloff = normalizedValue;
                logging::log << "Material Transparency Falloff: " << percentage << "%" << std::endl;
                break;
                
            case ChunkType::MAT_REFBLUR:
                targetMaterial_->reflectionBlur = normalizedValue;
                logging::log << "Material Reflection Blur: " << percentage << "%" << std::endl;
                break;
                
            case ChunkType::MAT_SELF_ILLUM:
                targetMaterial_->selfIllumination = normalizedValue;
                logging::log << "Material Self Illumination: " << percentage << "%" << std::endl;
                break;
                
            default:
                logging::log << "INT_PERCENTAGE (" << percentage << "%) in unknown context (parent: 0x" 
                         << std::hex << parentChunkId << std::dec << ")" << std::endl;
                break;
        }
    }
    
    bool FloatPercentageChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // Read percentage value as float
        float percentage;
        if (!Read(percentage)) {
            return false;
        }
        
        // Determine what property to set based on parent chunk
        SetMaterialProperty(percentage, parentId);
        
        return true;
    }
    
    void FloatPercentageChunk::SetMaterialProperty(float percentage, uint16_t parentChunkId) {
        float normalizedValue = percentage / 100.0f;
        
        switch (parentChunkId) {
            case ChunkType::MAT_SHININESS:
                targetMaterial_->shininess = normalizedValue * 128.0f;
                logging::log << "Material Shininess: " << percentage << "% (exponent " << targetMaterial_->shininess << ")" << std::endl;
                break;
                
            case ChunkType::MAT_SHIN2PCT:
                targetMaterial_->shininessPercent = normalizedValue;
                logging::log << "Material Specular Strength: " << percentage << "%" << std::endl;
                break;
                
            case ChunkType::MAT_TRANSPARENCY:
                targetMaterial_->transparency = normalizedValue;
                logging::log << "Material Transparency: " << percentage << "%" << std::endl;
                break;
                
            case ChunkType::MAT_XPFALL:
                targetMaterial_->transparencyFalloff = normalizedValue;
                logging::log << "Material Transparency Falloff: " << percentage << "%" << std::endl;
                break;
                
            case ChunkType::MAT_REFBLUR:
                targetMaterial_->reflectionBlur = normalizedValue;
                logging::log << "Material Reflection Blur: " << percentage << "%" << std::endl;
                break;
                
            case ChunkType::MAT_SELF_ILLUM:
                targetMaterial_->selfIllumination = normalizedValue;
                logging::log << "Material Self Illumination: " << percentage << "%" << std::endl;
                break;
                
            default:
                logging::log << "FLOAT_PERCENTAGE (" << percentage << "%) in unknown context (parent: 0x" 
                         << std::hex << parentChunkId << std::dec << ")" << std::endl;
                break;
        }
    }

} // namespace Debugger3DS::Parser::Chunks
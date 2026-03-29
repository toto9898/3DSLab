#pragma once

#include <cstdint>

namespace Debugger3DS {
    // 3DS file chunk type definitions from the 3DS format specification
    namespace ChunkType {
        // Main file chunks
        constexpr uint16_t M3DMAGIC        = 0x4D4D;  // Main chunk
        constexpr uint16_t M3D_VERSION     = 0x0002;  // Version
        constexpr uint16_t MDATA           = 0x3D3D;  // Mesh data
        constexpr uint16_t KFDATA          = 0xB000;  // Keyframe data
        
        // MDATA section chunks
        constexpr uint16_t MESH_VERSION    = 0x3D3E;  // Mesh version
        constexpr uint16_t MASTER_SCALE    = 0x0100;  // Master scale
        constexpr uint16_t AMBIENT_LIGHT   = 0x2100;  // Ambient light
        
        // Object chunks
        constexpr uint16_t NAMED_OBJECT    = 0x4000;  // Named object
        constexpr uint16_t N_TRI_OBJECT    = 0x4100;  // Triangle mesh object
        constexpr uint16_t POINT_ARRAY     = 0x4110;  // Vertex array
        constexpr uint16_t POINT_FLAG_ARRAY = 0x4111; // Vertex flags
        constexpr uint16_t FACE_ARRAY      = 0x4120;  // Face array
        constexpr uint16_t MSH_MAT_GROUP   = 0x4130;  // Material groups
        constexpr uint16_t TEX_VERTS       = 0x4140;  // Texture vertices
        constexpr uint16_t SMOOTH_GROUP    = 0x4150;  // Smoothing groups
        constexpr uint16_t MESH_MATRIX     = 0x4160;  // Mesh matrix
        constexpr uint16_t MESH_COLOR      = 0x4165;  // Mesh color
        constexpr uint16_t MESH_TEXTURE_INFO = 0x4170; // Texture info
        
        // Material chunks
        constexpr uint16_t MAT_ENTRY       = 0xAFFF;  // Material entry
        constexpr uint16_t MAT_NAME        = 0xA000;  // Material name
        constexpr uint16_t MAT_AMBIENT     = 0xA010;  // Ambient color
        constexpr uint16_t MAT_DIFFUSE     = 0xA020;  // Diffuse color
        constexpr uint16_t MAT_SPECULAR    = 0xA030;  // Specular color
        constexpr uint16_t MAT_SHININESS   = 0xA040;  // Shininess
        constexpr uint16_t MAT_SHIN2PCT    = 0xA041;  // Shininess percentage
        constexpr uint16_t MAT_TRANSPARENCY = 0xA050; // Transparency
        constexpr uint16_t MAT_XPFALL      = 0xA052;  // Transparency falloff
        constexpr uint16_t MAT_REFBLUR     = 0xA053;  // Reflection blur
        constexpr uint16_t MAT_SELF_ILLUM  = 0xA084;  // Self illumination
        constexpr uint16_t MAT_WIRE_SIZE   = 0xA087;  // Wire frame size
        constexpr uint16_t MAT_SHADING     = 0xA100;  // Shading type
        constexpr uint16_t MAT_TEXMAP      = 0xA200;  // Texture map
        constexpr uint16_t MAT_MAPNAME     = 0xA300;  // Map filename
        
        // Light chunks
        constexpr uint16_t N_DIRECT_LIGHT  = 0x4600;  // Direct light
        constexpr uint16_t DL_OFF          = 0x4620;  // Light off
        constexpr uint16_t DL_SPOTLIGHT    = 0x4610;  // Spotlight
        
        // Camera chunks
        constexpr uint16_t N_CAMERA        = 0x4700;  // Camera
        
        // Color chunks
        constexpr uint16_t COLOR_F         = 0x0010;  // Float color
        constexpr uint16_t COLOR_24        = 0x0011;  // 24-bit color
        constexpr uint16_t LIN_COLOR_24    = 0x0012;  // Linear 24-bit color
        constexpr uint16_t LIN_COLOR_F     = 0x0013;  // Linear float color
        
        // Percentage chunks
        constexpr uint16_t INT_PERCENTAGE  = 0x0030;  // Integer percentage
        constexpr uint16_t FLOAT_PERCENTAGE = 0x0031; // Float percentage
        
        // Keyframe chunks
        constexpr uint16_t KFHDR           = 0xB00A;  // Keyframe header
        constexpr uint16_t KFSEG           = 0xB008;  // Keyframe segment
        constexpr uint16_t KFCURTIME       = 0xB009;  // Current time
        constexpr uint16_t OBJECT_NODE_TAG = 0xB002;  // Object node
        constexpr uint16_t CAMERA_NODE_TAG = 0xB003;  // Camera node
        constexpr uint16_t LIGHT_NODE_TAG  = 0xB005;  // Light node
        constexpr uint16_t SPOTLIGHT_NODE_TAG = 0xB007; // Spotlight node
        constexpr uint16_t TARGET_NODE_TAG = 0xB004;  // Target node
        constexpr uint16_t L_TARGET_NODE_TAG = 0xB006; // Light target node
        
        // Node chunks
        constexpr uint16_t NODE_HDR        = 0xB010;  // Node header
        constexpr uint16_t INSTANCE_NAME   = 0xB011;  // Instance name
        constexpr uint16_t NODE_ID         = 0xB030;  // Node ID
        constexpr uint16_t PIVOT           = 0xB013;  // Pivot point
        constexpr uint16_t BOUNDBOX        = 0xB014;  // Bounding box
        constexpr uint16_t POS_TRACK_TAG   = 0xB020;  // Position track
        constexpr uint16_t ROT_TRACK_TAG   = 0xB021;  // Rotation track
        constexpr uint16_t SCL_TRACK_TAG   = 0xB022;  // Scale track
        constexpr uint16_t FOV_TRACK_TAG   = 0xB023;  // FOV track
        constexpr uint16_t ROLL_TRACK_TAG  = 0xB024;  // Roll track
        constexpr uint16_t COL_TRACK_TAG   = 0xB025;  // Color track
        constexpr uint16_t MORPH_TRACK_TAG = 0xB026;  // Morph track
        constexpr uint16_t HOT_TRACK_TAG   = 0xB027;  // Hotspot track
        constexpr uint16_t FALL_TRACK_TAG  = 0xB028;  // Falloff track
        constexpr uint16_t HIDE_TRACK_TAG  = 0xB029;  // Hide track
        constexpr uint16_t MORPH_SMOOTH    = 0xB032;  // Morph smooth
    }
}
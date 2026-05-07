#pragma once

#include <string>

namespace Debugger3DS::Scene {

/// @brief Base class for all scene objects that carry a name.
class NamedObject {
public:
    std::string name; ///< Human-readable object name (from the 3DS NAMED_OBJECT or NODE_HDR chunk).

    NamedObject() = default;
    /// @param objName Initial object name.
    NamedObject(const std::string& objName) : name(objName) {}
};

} // namespace Debugger3DS::Scene
#pragma once

#include <string>

namespace Debugger3DS {
    
    class NamedObject {
    public:
        std::string name;

        NamedObject() = default;
        NamedObject(const std::string& objName) : name(objName) {}
    };
    
} // namespace Debugger3DS
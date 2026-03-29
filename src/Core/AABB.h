#pragma once

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <limits>

namespace Debugger3DS {

// Axis-aligned bounding box
struct AABB {
    Eigen::Vector3d min;
    Eigen::Vector3d max;
    
    AABB() : min(Eigen::Vector3d::Zero()), max(Eigen::Vector3d::Zero()) {}
    AABB(const Eigen::Vector3d& minPt, const Eigen::Vector3d& maxPt) : min(minPt), max(maxPt) {}
    
    // Check if ray intersects this bounding box
    bool IntersectsRay(const Eigen::Vector3d& origin, const Eigen::Vector3d& direction, double& tMin, double& tMax) const {
        // Ray-AABB intersection using slab method
        tMin = 0.0;
        tMax = std::numeric_limits<double>::max();
        
        for (int i = 0; i < 3; ++i) {
            if (std::abs(direction(i)) < 1e-8) {
                if (origin(i) < min(i) || origin(i) > max(i)) {
                    return false;
                }
            } else {
                double t1 = (min(i) - origin(i)) / direction(i);
                double t2 = (max(i) - origin(i)) / direction(i);
                
                if (t1 > t2) std::swap(t1, t2);
                
                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);
                
                if (tMin > tMax) {
                    return false;
                }
            }
        }
        
        return true;
    }
};

} // namespace Debugger3DS

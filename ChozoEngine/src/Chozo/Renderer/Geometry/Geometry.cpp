#include "Geometry.h"

#include "BoxGeometry.h"
#include "SphereGeometry.h"

namespace Chozo
{

    Geometry Geometry::Create(const GeometryType type)
    {
        switch (type)
        {
            case GeometryType::Plane:
            {
                return Geometry();
            }
            case GeometryType::Box:
            {
                return BoxGeometry();
            }
            case GeometryType::Sphere:
            {
                return SphereGeometry();
            }
            case GeometryType::Cone:
            {
                return Geometry();
            }
            case GeometryType::Cylinder:
            {
                return Geometry();
            }
        }
    }
}

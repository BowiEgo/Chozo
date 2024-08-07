#include "Geometry.h"

#include "BoxGeometry.h"
#include "SphereGeometry.h"

namespace Chozo
{

    Ref<Geometry> Geometry::Create(const GeometryType type)
    {
        switch (type)
        {
            case GeometryType::Plane:
            {
                return std::make_shared<Geometry>();
            }
            case GeometryType::Box:
            {
                return std::make_shared<BoxGeometry>();
            }
            case GeometryType::Sphere:
            {
                return std::make_shared<SphereGeometry>();
            }
            case GeometryType::Cone:
            {
                return std::make_shared<Geometry>();
            }
            case GeometryType::Cylinder:
            {
                return std::make_shared<Geometry>();
            }
        }
    }
}

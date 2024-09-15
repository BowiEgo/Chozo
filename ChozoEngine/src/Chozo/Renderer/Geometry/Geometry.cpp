#include "Geometry.h"

#include "BoxGeometry.h"
#include "SphereGeometry.h"
#include "QuadGeometry.h"

namespace Chozo
{

    // Ref<Geometry> Geometry::Create(const GeometryType type, GeometryProps props)
    // {
    //     switch (type)
    //     {
    //         case GeometryType::Plane:
    //         {
    //             return std::make_shared<Geometry>();
    //         }
    //         case GeometryType::Box:
    //         {
    //             BoxGeometryProps* boxProps = dynamic_cast<BoxGeometryProps*>(&props);
    //             return std::make_shared<BoxGeometry>(
    //                 boxProps->Width,
    //                 boxProps->Height,
    //                 boxProps->Depth,
    //                 boxProps->WidthSegments,
    //                 boxProps->HeightSegments,
    //                 boxProps->DepthSegments
    //             );
    //         }
    //         case GeometryType::Sphere:
    //         {
    //             return std::make_shared<SphereGeometry>(props);
    //         }
    //         case GeometryType::Cone:
    //         {
    //             return std::make_shared<Geometry>();
    //         }
    //         case GeometryType::Cylinder:
    //         {
    //             return std::make_shared<Geometry>();
    //         }
    //     }
    // }
}

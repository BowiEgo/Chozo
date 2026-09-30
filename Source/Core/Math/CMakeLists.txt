set(MODULE_NAME CZMath)

set(MODULE_TYPE Core)

set(MODULE_INCLUDE
	${CHOZO_INCLUDE_DIR}/Core/Math/MathUtils.hpp
	${CHOZO_INCLUDE_DIR}/Core/Math/Matrix3.hpp
	${CHOZO_INCLUDE_DIR}/Core/Math/Matrix4.hpp
	${CHOZO_INCLUDE_DIR}/Core/Math/Quaternion.hpp
	${CHOZO_INCLUDE_DIR}/Core/Math/Vector2.hpp
	${CHOZO_INCLUDE_DIR}/Core/Math/Vector3.hpp
	${CHOZO_INCLUDE_DIR}/Core/Math/Vector4.hpp
)

set(MODULE_SRC
	MathUtils.cpp
	Matrix3.cpp
	Matrix4.cpp
	Quaternion.cpp
	Vector2.cpp
	Vector3.cpp
	Vector4.cpp
)

# Precompiled header: the module ships MathPCH.h next to its sources, and the module function
# picks up MODULE_PCH from here. Explicit rather than inferred, so which modules have one is
# visible in their own file.
set(MODULE_PCH "${CMAKE_CURRENT_SOURCE_DIR}/MathPCH.h")

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    glm::glm
    PCH     MathPCH.h
)

add_chozo_module_tests()

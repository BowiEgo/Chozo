// These variables should be in a struct but some GPU drivers ignore the
// precision qualifier on individual struct members
highp mat3  shading_tangentToWorld;   // TBN matrix
highp vec3  shading_position;         // position of the fragment in world space
      vec3  shading_view;             // normalized vector from the fragment to the eye
      vec3  shading_normal;           // normalized transformed normal, in world space
      vec3  shading_geometricNormal;  // normalized geometric normal, in world space
      vec3  shading_reflected;        // reflection of view about normal
      float shading_NoV;              // dot(normal, view), always strictly >= MIN_N_DOT_V

highp vec2 shading_normalizedViewportCoord;

void computeShadingParams()
{
      highp vec3 n = vertex_worldNormal;
      highp vec3 t = vertex_worldTangent.xyz;
      highp vec3 b = cross(n, t) * sign(vertex_worldTangent.w);

      shading_geometricNormal = normalize(n);
      shading_tangentToWorld = mat3(t, b, n);
      shading_position = vertex_worldPosition.xyz;

      // With perspective camera, the view vector is cast from the fragment pos to the eye position,
      // With ortho camera, however, the view vector is the same for all fragments:
      highp vec3 sv = isPerspectiveProjection() ?
            (frameUniforms.worldFromViewMatrix[3].xyz - shading_position) :
            frameUniforms.worldFromViewMatrix[2].xyz; // ortho camera backward dir
      shading_view = normalize(sv);

      shading_normalizedViewportCoord = vertex_position.xy * (0.5 / vertex_position.w) + 0.5;
}

/**
 * Computes global shading parameters that the material might need to access
 * before lighting: N dot V, the reflected vector and the shading normal (before
 * applying the normal map). These parameters can be useful to material authors
 * to compute other material properties.
 *
 * This function must be invoked by the user's material code (guaranteed by
 * the material compiler) after setting a value for MaterialInputs.normal.
 */
void prepareMaterial(const MaterialInputs material)
{
#if defined(MATERIAL_HAS_NORMAL)
    shading_normal = normalize(shading_tangentToWorld * material.normal);
#else
    shading_normal = getWorldGeometricNormalVector();
#endif
    shading_NoV = clampNoV(dot(shading_normal, shading_view));
    shading_reflected = reflect(-shading_view, shading_normal);
}
vec4 modelPosition = u_VertUniforms.ModelMatrix * vec4(a_Position, 1.0);
vec4 viewPosition = u_ViewMatrix * modelPosition;
vec4 projectionPosition = u_ProjectionMatrix * viewPosition;

gl_Position = projectionPosition;

v_Normal = a_Normal;
v_TexCoord = a_TexCoord;
v_FragPosition = vec3(modelPosition);
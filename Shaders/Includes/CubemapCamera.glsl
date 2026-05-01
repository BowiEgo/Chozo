// layout(set = 0, binding = 0) uniform CubemapCaptureData {
//     mat4 ViewMatrix;
//     mat4 ProjMatrix;
// } u_CaptureCamera;

layout(push_constant) uniform PC {
    mat4 ViewMatrix;
    mat4 ProjMatrix;
} u_CaptureCamera;
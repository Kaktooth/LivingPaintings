#version 460
#if VULKAN
layout(set = 0, binding = 0) uniform InstanceUbo {
    mat4 model;
} uboInstance;

layout(binding = 1) uniform ViewUbo {
    mat4 view;
    mat4 proj;
} uboView;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec3 position;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out vec3 cameraView;
layout(location = 3) out mat4 model;
layout(location = 7) out int instanceID;

void main() {
	gl_Position = uboView.proj * uboView.view * uboInstance.model * vec4(inPosition, 1.0f);
    position = inPosition;
    texCoord = inTexCoord;
    cameraView = uboView.view[2].xyz;
    model = uboInstance.model;
    instanceID = gl_InstanceIndex;
}
#endif
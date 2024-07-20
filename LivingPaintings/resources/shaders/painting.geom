#version 460

layout(triangles) in;
layout(location = 0) in vec2 inTexCoord[];
layout(location = 1) in vec3 inCameraView[];
layout(location = 2) in mat4 model[];

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 cameraView;
layout(location = 2) out vec3 normal;
layout(triangle_strip, max_vertices = 3) out;

void main() {
	vec3 v0 = gl_in[0].gl_Position.xyz;
	vec3 v1 = gl_in[1].gl_Position.xyz;
	vec3 v2 = gl_in[2].gl_Position.xyz;

	vec3 u = v1 - v0;
	vec3 v = v2 - v0;

	normal = normalize(cross(u, v));
	normal = vec3(
		normal.x + dot(normal.x, model[0][1].x),
		normal.y + dot(normal.y, model[0][1].y),
		normal.z + dot(normal.z, model[0][1].z)
	);

	for(int i = 0; i < gl_in.length(); i++) {
		cameraView = inCameraView[i];
		fragTexCoord = inTexCoord[i];
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
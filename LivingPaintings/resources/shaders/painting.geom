#version 460

layout(triangles) in;
layout(location = 0) in vec3 inPositions[];
layout(location = 1) in vec2 inTexCoord[];
layout(location = 2) in vec3 inCameraView[];
layout(location = 3) in mat4 model[];

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 cameraView;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec3 outTangentViewPos;
layout(location = 4) out vec3 outTangentFragPos;
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

	vec2 s = vec2(inTexCoord[1].x - inTexCoord[0].x, inTexCoord[2].x - inTexCoord[0].x);
	vec2 t = vec2(inTexCoord[1].y - inTexCoord[0].y, inTexCoord[2].y - inTexCoord[0].y);

	float r = 1.0f / (s.x * t.y - s.y * t.x);
	mat2x3 TB = mat2x3((t.y * u.x - t.x * v.x) * r, (t.y * u.y - t.x * v.y) * r, (t.y * u.z - t.x * v.z) * r,
					   (s.x * v.x - s.y * u.x) * r, (s.x * v.y - s.y * u.y) * r, (s.x * v.z - s.y * u.z) * r);

	vec3 tangent = normalize(TB[0] - normal * dot(normal, TB[0].xyz)); 

	vec3 N = normalize(mat3(model[0]) * normal);
	vec3 T = normalize(mat3(model[0]) * TB[0]);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = transpose(mat3(T, B, N));

	outTangentViewPos = TBN * inCameraView[0];
	outTangentFragPos = TBN * vec3(model[0] * vec4(inPositions[0], 1.0f));

	for(int i = 0; i < gl_in.length(); i++) {
		cameraView = inCameraView[i];
		fragTexCoord = inTexCoord[i];
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
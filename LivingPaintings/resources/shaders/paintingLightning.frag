#version 460

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 3) uniform sampler2D heightMapTexSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 cameraView;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 outFragColor;

const float gamma = 2.2f;
const vec3 luma = vec3(0.2627f, 0.678f, 0.0593f);

// scale factor that is used to vary the range of the height values
const float heightRange = 1.5f;

const float surfaceColorModifier = 0.45f;
const vec3 colorPallete [2] = { vec3(0.05f, 0.052f, 0.074f), vec3(0.0047f, 0.0094f, 0.026f) };

void main() {

	vec3 texColor = texture(texSampler, fragTexCoord).rgb;

//* Gamma correction
	texColor = pow(texColor, vec3(1.0f / gamma));
	float luminance = dot(texColor, luma);

//* Shading

//	Caculate normals of height texture
	float right = texture(heightMapTexSampler, vec2(fragTexCoord.x + 1, fragTexCoord.y)).r;
	float left = texture(heightMapTexSampler, vec2(fragTexCoord.x - 1, fragTexCoord.y)).r;
	float up = texture(heightMapTexSampler, vec2(fragTexCoord.x, fragTexCoord.y - 1)).r;
	float down = texture(heightMapTexSampler, vec2(fragTexCoord.x, fragTexCoord.y + 1)).r;
	vec3 N = vec3(heightRange * right - heightRange * left, heightRange * up - heightRange * down, 1);
	N = N / length(N); 


//  Gooch Illumination Model Calculation
	vec3 surfaceColor = surfaceColorModifier * texColor;
	vec3 coolColor = colorPallete[0] + surfaceColor;
	vec3 warmColor = colorPallete[1] + surfaceColor;
	vec3 lightPos = cameraView + vec3(50.0f, 50.0f, 5.0f);
	float angle = dot(normal + N, lightPos);
	float t = (angle + 1) / 2;
	vec3 reflectedLight = reflect(normal + N, lightPos);  
	float highlightBlendFactor = clamp(100 * dot(reflectedLight, cameraView) - 97, 0, 1);
	vec3 shaded = highlightBlendFactor * warmColor + (1 - highlightBlendFactor)
	* (t * warmColor + ((1 - t) * coolColor));

	outFragColor = vec4(texColor + shaded, 1.0);
}
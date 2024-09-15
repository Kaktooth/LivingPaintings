#version 460

layout(binding = 2) uniform sampler2D texSampler;
layout(binding = 4) uniform sampler2D heightMapTexSampler;
layout(binding = 5) uniform MouseControls {
    dvec2 mousePos;
	ivec2 windowSize;
    float squareSize;
    bool pixelScaling;
} mouseControl;
layout(binding = 6) uniform sampler2D selectedPositionsMask;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 cameraView;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 inTangentViewPos;
layout(location = 4) in vec3 inTangentFragPos;

layout(location = 0) out vec4 outFragColor;

// Parallax mapping parameters
const float heightScale = 0.005f;
const float parallaxBias = 0.001f;

const float gamma = 2.2f;
const vec3 luma = vec3(0.2627f, 0.678f, 0.0593f);

// Scale factor that is used to vary the range of the height values
const float heightRange = 0.5f;

const float surfaceColorModifier = 0.35f;
const vec3 colorPallete [2] = { vec3(0.06f, 0.052f, 0.074f), vec3(0.0047f, 0.0094f, 0.026f) };

const float textureScale = 0.2f;

void main() {

//	Calculate Parallax mapping
	vec3 viewDirection = normalize(inTangentViewPos - inTangentFragPos);
	float height = 1.0f - texture(heightMapTexSampler, fragTexCoord).r;
	vec2 p =  (height * (heightScale * 0.5f) + parallaxBias) * viewDirection.xy;
	vec2 UV = fragTexCoord - p;

	vec2 texSize = textureSize(texSampler, 1);
	vec3 texColor = texture(texSampler, UV).rgb;

//* Gamma correction
	texColor = pow(texColor, vec3(1.0f / gamma));
	float luminance = dot(texColor, luma);

//* Shading

//	Caculate normal map with height map using centered difference approximations
	float right = texture(heightMapTexSampler, vec2(fragTexCoord.x + 1. / texSize.x, fragTexCoord.y)).r;
	float left = texture(heightMapTexSampler, vec2(fragTexCoord.x - 1. / texSize.x, fragTexCoord.y)).r;
	float up = texture(heightMapTexSampler, vec2(fragTexCoord.x, fragTexCoord.y -  1. / texSize.y)).r;
	float down = texture(heightMapTexSampler, vec2(fragTexCoord.x, fragTexCoord.y + 1. / texSize.y)).r;

	float hx = (right - left) / 2;
	float hy = (up - down) / 2;
	vec3 N = vec3(-hx * heightRange, -hy * heightRange, 1);
//  Compress vector to range from [-1, 1] to [0, 1]
	N = vec3((N.x + 1) / 2, (N.y + 1) / 2, (N.z + 1) / 2);

//  Gooch Illumination Model Calculation
	vec3 norm = normal + N;
	vec3 surfaceColor = surfaceColorModifier * texColor;
	vec3 coolColor = colorPallete[0] + surfaceColor;
	vec3 warmColor = colorPallete[1] + surfaceColor;
	vec3 lightPos = cameraView + vec3(20.0f, 20.0f, 5.0f);
	float angle = dot(norm, lightPos);
	float t = (angle + 1) / 2;
	vec3 reflectedLight = reflect(norm, lightPos);  
	float highlightBlendFactor = clamp(100 * dot(reflectedLight, cameraView) - 97, 0, 1);
	vec3 shaded = highlightBlendFactor * warmColor + (1 - highlightBlendFactor)
	* (t * warmColor + ((1 - t) * coolColor));

	vec4 paintingOutput = vec4(texColor + shaded, 1.0);
   	outFragColor = paintingOutput;
	
//* Pixels zoom in

	//	If enabled, construct square with scaled texture content
	if(mouseControl.pixelScaling) {
	    vec2 mousePos = vec2(mouseControl.mousePos / mouseControl.windowSize);
		vec2 uv = fragTexCoord;
		uv -= fragTexCoord + clamp(mousePos - fragTexCoord, -1.0f, 1.0f);
		uv.x *= texSize.x / texSize.y;
		vec2 minDistance = step(-mouseControl.squareSize, uv);
		vec2 maxDistance = step(mouseControl.squareSize, uv);
		float squareDistance = (minDistance.x - maxDistance.x) * (minDistance.y - maxDistance.y);
		vec3 square = vec3(squareDistance);

		vec2 texUV = (((fragTexCoord - mousePos) * textureScale) + (mousePos * textureScale) / textureScale);

		vec4 scaledTex = texture(texSampler, texUV).rgba;
		vec4 selectedPosColor = texture(selectedPositionsMask, texUV).rrra;
		vec3 selectedPosColor2 = texture(selectedPositionsMask, fragTexCoord).rrr;

		scaledTex.rgb *= square;
		scaledTex.rgb -= selectedPosColor2;
		outFragColor = mix(scaledTex + selectedPosColor, outFragColor, 1.0f - square.r);

		//	Make pointer 
		vec2 minBrushDistance = step(-mouseControl.squareSize * textureScale * 0.3f, uv);
		vec2 maxBrushDistance = step(mouseControl.squareSize * textureScale * 0.3f, uv);
		float brushDistance = (minBrushDistance.x - maxBrushDistance.x) * (minBrushDistance.y - maxBrushDistance.y);
		vec3 pointerBrush = vec3(brushDistance);
	    outFragColor = outFragColor + vec4(pointerBrush, 1.0f);
	}
	
	vec3 selectedPosColor = texture(selectedPositionsMask, fragTexCoord).rrr;
	outFragColor = outFragColor + vec4(selectedPosColor, 1.0f);
	
}
#version 460

precision highp float;

const uint MASKS_COUNT = 4;
const uint EFFECTS_COUNT = MASKS_COUNT - 1;
const uint PACKED_EFFECTS_COUNT = (EFFECTS_COUNT + 3) / 4;

layout(binding = 2) uniform sampler2D texSampler;
layout(binding = 4) uniform sampler2D heightMapTexSampler;
layout(binding = 5) uniform MouseControls {
    dvec2 mousePos;
	ivec2 windowSize;
    float squareSize;
    bool pixelScaling;
	int maskEffectIndex;
} mouseControl;
layout(binding = 6) uniform sampler2D selectedPositionsMask[MASKS_COUNT];
layout(binding = 7) uniform RuntimeProperties {
	float time;
} prop;
layout(binding = 8) uniform EffectParams {
    uvec4 enabledEffects[PACKED_EFFECTS_COUNT];
	uint highlightSelectedPixels;
	float noiseScale;
	float distortionModifier;
	float heightScale;
	float amplifyFlickeringLight;
	float amplifyHighlight;
} effectsParams;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 cameraView;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 inTangentViewPos;
layout(location = 4) in vec3 inTangentFragPos;

layout(location = 0) out vec4 outFragColor;

const vec3 maskColors[MASKS_COUNT] = { vec3(0.7f), vec3(0.5f, 1.0f, 0.4f), vec3(0.01f, 0.01f, 1.26f), vec3(0.97f, 0.011f, 0.26f) };
vec2 noiseScale = vec2(effectsParams.noiseScale);
float heightBias = effectsParams.heightScale * -0.5f;

// Scale factor that is used to vary the range of the height values
const float heightRange = 0.5f;

const float gamma = 2.2f;
const vec3 luma = vec3(0.2627f, 0.678f, 0.0593f);

const float surfaceColorModifier = 0.35f;
const vec3 colorPallete [2] = { vec3(0.06f, 0.052f, 0.074f), vec3(0.0047f, 0.0094f, 0.026f) };

const float textureScale = 0.2f;

float random (vec2 fragPos) {
	return fract(sin(dot(fragPos.xy, vec2(54.236, 98.987))) * 43757.5386029);
}

// 2D Perlin Noise from https://thebookofshaders.com/11
float perlinNoise(vec2 fragPos, vec2 scale) {
	vec2 scaledfragTexCoord = fragPos * scale;
	vec2 i = vec2(floor(scaledfragTexCoord));
	vec2 f = vec2(fract(scaledfragTexCoord));
    vec2 u = smoothstep(0.,1.,f);

	float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

void main() {
//	Calculate Parallax mapping
	vec3 viewDirection = normalize(inTangentViewPos - inTangentFragPos);
	float biasedHeight = texture(heightMapTexSampler, fragTexCoord).r * effectsParams.heightScale + heightBias;
	vec2 p = viewDirection.xy * biasedHeight;
	vec2 UV = fragTexCoord - p;

//* Animation
// 	Animate selected objects that was selected.
	float selectedObjectMask = texture(selectedPositionsMask[0], fragTexCoord).r;
	float distortionMask = texture(selectedPositionsMask[1], fragTexCoord).r;
	float flickeringMask = texture(selectedPositionsMask[2], fragTexCoord).r;
	float highlightMask = texture(selectedPositionsMask[3], fragTexCoord).r;

	vec3 mixColor = vec3(0.0f);
	float noise = perlinNoise(fragTexCoord, noiseScale);
	// Effect for object construction mask 

	// Slow distortion effect
	if(distortionMask != 0 && effectsParams.enabledEffects[0][0] == 1) {
		UV.x += sin(prop.time) * noise * effectsParams.distortionModifier;
		UV.y += sin(prop.time) * noise * effectsParams.distortionModifier;
	}
	// Flickering effect for object construction mask 
	if(flickeringMask != 0 && effectsParams.enabledEffects[0][1] == 1) {
		mixColor += mix(vec3(0.0f), maskColors[0], abs(sin(prop.time)) * noise) * effectsParams.amplifyFlickeringLight;
	}
	// Highlight effect
	if(highlightMask != 0 && effectsParams.enabledEffects[0][2] == 1) {
		mixColor += mix(maskColors[1], maskColors[2], abs(sin(prop.time)) * noise) * effectsParams.amplifyHighlight;
	}

	vec2 texSize = textureSize(texSampler, 1);
	vec3 texColor = texture(texSampler, UV).rgb + mixColor;

//* Gamma correction
	texColor = pow(texColor, vec3(1.0f / gamma));
	float luminance = dot(texColor, luma);

//* Shading

//	Caculate normal map with height map using centered difference approximations
	vec2 stepRight = vec2(UV.x + 1. / texSize.x, UV.y);
	vec2 stepLeft = vec2(UV.x - 1. / texSize.x, UV.y);
	vec2 stepUp = vec2(UV.x, UV.y -  1. / texSize.y);
	vec2 stepDown = vec2(UV.x, UV.y + 1. / texSize.y);
	float right = texture(heightMapTexSampler, stepRight).r;
	float left = texture(heightMapTexSampler, stepLeft).r;
	float up = texture(heightMapTexSampler, stepUp).r;
	float down = texture(heightMapTexSampler, stepDown).r;

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
		vec4 selectedPosColor = texture(selectedPositionsMask[0], texUV).rrra;
		vec3 revertPosColor = texture(selectedPositionsMask[0], fragTexCoord).rrr;

		scaledTex.rgb *= square;
		scaledTex.rgb -= revertPosColor;
		outFragColor = mix(scaledTex + selectedPosColor, outFragColor, 1.0f - square.r);

		//	Make pointer 
		vec2 minBrushDistance = step(-mouseControl.squareSize * textureScale * 0.3f, uv);
		vec2 maxBrushDistance = step(mouseControl.squareSize * textureScale * 0.3f, uv);
		float brushDistance = (minBrushDistance.x - maxBrushDistance.x) * (minBrushDistance.y - maxBrushDistance.y);
		vec3 pointerBrush = vec3(brushDistance);
	    outFragColor = outFragColor + vec4(pointerBrush, 1.0f);
	}
	
	//Highlight every selected pixel in the masks
	if(effectsParams.highlightSelectedPixels == 1) {
		vec3 selectedPosColor = vec3(0);
		for(uint maskIndex = 0; maskIndex < MASKS_COUNT; maskIndex++) {
			selectedPosColor += texture(selectedPositionsMask[maskIndex], fragTexCoord).rrr * maskColors[maskIndex];
		}
		outFragColor = outFragColor + vec4(selectedPosColor, 1.0f);
	}
}
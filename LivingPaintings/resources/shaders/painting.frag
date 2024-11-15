#version 460
#extension GL_EXT_nonuniform_qualifier : enable

precision highp float;

const uint EFFECTS_COUNT = 3;
const uint MASKS_COUNT = EFFECTS_COUNT + 1;
const uint PACKED_EFFECTS_COUNT = (EFFECTS_COUNT + 3) / 4;

layout(set = 1, binding = 0) uniform sampler2D paintingTexSampler[];
layout(binding = 3) uniform sampler2D heightMapTexSampler;
layout(binding = 4) uniform MouseControls {
    dvec2 mousePos;
	ivec2 windowSize;
    float squareSize;
    bool pixelScaling;
	int maskEffectIndex;
} mouseMaskControl;
layout(binding = 5) uniform sampler2D selectedPositionsMask[MASKS_COUNT];
layout(binding = 6) uniform RuntimeProperties {
	float time;
} prop;
layout(binding = 7) uniform EffectParams {
    uvec4 enabledEffects[PACKED_EFFECTS_COUNT];
	uint highlightSelectedPixels;
    float heightRange; // Scale factor that is used to vary the range of the height values
	float noiseScale;
	float distortionModifier;
	float parallaxHeightScale;
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

const float gamma = 2.2f;
const float surfaceColorModifier = 0.35f;
const vec3 colorPallete [2] = { vec3(0.06f, 0.052f, 0.074f), vec3(0.0047f, 0.0094f, 0.026f) };

const float textureScale = 0.2f;

vec2 random2(vec2 st){
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
float gradientNoise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f); // cubic Hermit curve

    return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}


/*
  Implementation of Parallax Mapping with Offset Limiting. This is done by calculating the camera's direction
  to the surface and finding the height bias, where the parallaxHeightScale parameter will be multiplied by -0.5
  so that the height can include zero values, since the surface will intersect with the polygon that will display it.

  Next, the height will be calculated, where the product of the height obtained from the height map with the 
  parallaxHeightScale parameter and the height bias will be found. Next, the offset for the texture coordinates 
  will be found by finding the product of the camera direction to the painting surface and the height. After that,
  the texture coordinates will be shifted by subtracting the offset from the texture coordinates, so that larger
  height values are shifted faster than smaller ones, where the height values are in the range from 0 to 1.

  Parameters:
	viewDirection - tangent-space viewing direction.

  References:
        [T. Welsh, 2004] "Parallax Mapping with Offset Limiting: A Per­Pixel Approximation of Uneven Surfaces"
*/
vec2 parallaxMapping(vec3 viewDirection) {
	float heightBias = effectsParams.parallaxHeightScale * -0.5f;
	float biasedHeight = texture(heightMapTexSampler, fragTexCoord).r * effectsParams.parallaxHeightScale + heightBias;
	vec2 P = viewDirection.xy * biasedHeight;
	return fragTexCoord - P;
}

/*
  Parallax Occlusion Mapping implementation. Firstly, view direction is used to calculate parallax 
  offset P. Next, ray-cast the view ray along parallax offset vector P calculating layer depth that 
  is an estimated depth of ray until ray is touched and moving texture coordinates with parallax 
  offset P. After ray is touched height of texel loop will break and texture coordinates will be found.
  
  Parameters:
	viewDirection - tangent-space viewing direction.
	layerNumber   - is the number of layers used to control the number of samples that pass 
	through the view ray along P in the direction of view (viewDirection).

  References:
        [N. Tatarchuk, 2006] "Practical Parallax Occlusion Mapping with Approximate Soft Shadows for
							  Detailed Surface Rendering"
*/
vec2 parallaxOcclusionMapping(vec3 viewDirection, float layerNumber) {
	float layerDepth = 1.0f / layerNumber;
	float currLayerDepth = 0.0f;
	vec2 P = viewDirection.xy * (effectsParams.parallaxHeightScale / 100);
	vec2 currUV = fragTexCoord;
	float height = 1.0f - texture(heightMapTexSampler, currUV).r;
	for (float depth = 1; depth >= -0.1f; depth -= layerDepth) {
		currLayerDepth += layerDepth;
		currUV -= P;
		height = 1.0f - texture(heightMapTexSampler, currUV).r;
		if (height < currLayerDepth) {
			break;
		}
	}
	vec2 prevUV = currUV + P;
	float nextDepth = height - currLayerDepth;
	float prevDepth = 1.0f - texture(heightMapTexSampler, prevUV).r - currLayerDepth + layerDepth;
	return mix(currUV, prevUV, nextDepth / (nextDepth - prevDepth));
}

void main() {
//	Calculate Parallax mapping
	vec3 viewDirection = normalize(inTangentViewPos - inTangentFragPos);
	vec2 UV = parallaxOcclusionMapping(viewDirection, 50.0f);

 	// Create animations or effects for selected objects that was selected.
	float selectedObjectMask = texture(selectedPositionsMask[0], fragTexCoord).r;
	float distortionMask = texture(selectedPositionsMask[1], fragTexCoord).r;
	float flickeringMask = texture(selectedPositionsMask[2], fragTexCoord).r;
	float highlightMask = texture(selectedPositionsMask[3], fragTexCoord).r;

	vec3 mixColor = vec3(0.0f);
	vec2 noiseScale = vec2(effectsParams.noiseScale);
	vec2 scaledFragTexCoord = fragTexCoord * noiseScale;
	float noise = gradientNoise(scaledFragTexCoord);
	// Effect for object construction mask 

	// Sway effect
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

	vec2 texSize = textureSize(paintingTexSampler[gl_Layer], 1);
	vec3 texColor = texture(paintingTexSampler[gl_Layer], UV).rgb + mixColor;

//* Gamma correction
	texColor = pow(texColor, vec3(1.0f / gamma));

//* Shading
 
//	Caculate normal map with height map using centered difference approximations
	vec2 stepRight = vec2(UV.x + 1. / texSize.x, UV.y);
	vec2 stepLeft = vec2(UV.x - 1. / texSize.x, UV.y);
	vec2 stepUp = vec2(UV.x, UV.y - 1. / texSize.y);
	vec2 stepDown = vec2(UV.x, UV.y + 1. / texSize.y);
	float right = texture(heightMapTexSampler, stepRight).r;
	float left = texture(heightMapTexSampler, stepLeft).r;
	float up = texture(heightMapTexSampler, stepUp).r;
	float down = texture(heightMapTexSampler, stepDown).r;

	float hx = (right - left) / 2;
	float hy = (up - down) / 2;
	vec3 N = vec3(-hx * effectsParams.heightRange, -hy * effectsParams.heightRange, 1);
//  Compress vector from range [-1, 1] to range [0, 1]
	N = vec3((N.x + 1) / 2, (N.y + 1) / 2, 1);

//  Gooch Illumination Model Calculation
	vec3 norm = normal + N;
	vec3 surfaceColor = surfaceColorModifier * texColor;
	vec3 coolColor = colorPallete[0] + surfaceColor;
	vec3 warmColor = colorPallete[1] + surfaceColor;
	vec3 lightPos = cameraView + vec3(0.0f, 10.0f, 5.0f);
	float angle = dot(norm, lightPos);
	vec3 reflectedLight = reflect(norm, lightPos);  
	float highlightBlendFactor = clamp(100 * dot(reflectedLight, cameraView) - 97, 0, 1);
	float t = (angle + 1) / 2;
	vec3 shaded = highlightBlendFactor * warmColor + (1 - highlightBlendFactor)
	* (t * warmColor + ((1 - t) * coolColor));

	vec4 paintingOutput = vec4(texColor + shaded, 1.0);
   	outFragColor = paintingOutput;

	//	If enabled, construct square with scaled texture content
	if(mouseMaskControl.pixelScaling) {
	    vec2 mousePos = vec2(mouseMaskControl.mousePos / mouseMaskControl.windowSize);
		vec2 uv = fragTexCoord;
		uv -= fragTexCoord + clamp(mousePos - fragTexCoord, -1.0f, 1.0f);
		uv.x *= texSize.x / texSize.y;
		vec2 minDistance = step(-mouseMaskControl.squareSize, uv);
		vec2 maxDistance = step(mouseMaskControl.squareSize, uv);
		float squareDistance = (minDistance.x - maxDistance.x) * (minDistance.y - maxDistance.y);
		vec3 square = vec3(squareDistance);

		vec2 texUV = (((fragTexCoord - mousePos) * textureScale) + (mousePos * textureScale) / textureScale);

		vec4 scaledTex = texture(paintingTexSampler[gl_Layer], texUV).rgba;
		vec4 selectedPosColor = texture(selectedPositionsMask[0], texUV).rrra;
		vec3 revertPosColor = texture(selectedPositionsMask[0], fragTexCoord).rrr;
		
		scaledTex.rgb *= square;
		scaledTex.rgb -= revertPosColor;
		outFragColor = mix(scaledTex + selectedPosColor, outFragColor, 1.0f - square.r);

		//	Make pointer for the cursor
		vec2 minBrushDistance = step(-mouseMaskControl.squareSize * textureScale * 0.3f, uv);
		vec2 maxBrushDistance = step(mouseMaskControl.squareSize * textureScale * 0.3f, uv);
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
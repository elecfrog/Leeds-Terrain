#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertPosition_modelspace;
layout(location = 1) in vec2 vertUV;
layout(location = 2) in vec3 vertNormal_modelspace;


out VERT_DATA
{
    vec2 heihgtRadio;
    vec4 MVP_Position;
}vertOut;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 M;
uniform mat3 MV3x3;
uniform vec3 LightPosition_worldspace;

uniform mat4 V;
uniform mat4 P;
// Values that stay constant for the whole mesh.
uniform sampler2D DiffuseTextureSampler;

// Using Bit Opertions to Get Real Height Value From [0,255] RGB Value
float compute_height(vec3 RGBValue, float scale, float shift)
{
	int height = int(int(RGBValue.r) << 16) + int(int(RGBValue.g) << 8)  + int(RGBValue.b);

	return scale * float(height) + shift;
}

void main()
{
	float y_scale = 0.00002f;
	float y_shift = -50.0f;
	float y_length = 10.0f;
	vec3 heightRGB = texture(DiffuseTextureSampler, vec2(vertUV.x, vertUV.y)).rgb * 255.0f;
	float real_height = compute_height(heightRGB, y_scale, y_shift);

    // gl_Position = MVP * vec4(vertPosition_modelspace.x, real_height, vertPosition_modelspace.z, 1.0f); // Matrix transformations go here
    vertOut.MVP_Position = MVP * vec4(vertPosition_modelspace.x, real_height, vertPosition_modelspace.z, 1.0f); 
    vertOut.heihgtRadio = vec2(0,0);

    float upheight = real_height - y_shift;
	if( real_height > -50.0f && real_height < -30.0f)
	{
		vertOut.heihgtRadio.x = (1 - upheight/20.0f);
		vertOut.heihgtRadio.y = upheight/20.0f;
	}
	else
		vertOut.heihgtRadio = vec2(0.0f,1.0f);
}
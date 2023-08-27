#version 330 core
in vec2 fragTexCoords;		// Simply Output UV coordinates for new triangles texture rendering
in vec2 fragHeihgtRadio;

// Values that stay constant for the whole mesh.
uniform sampler2D DiffuseTextureSampler;

// Ouput data
out vec4 color;

void main()
{
    if(fragHeihgtRadio.x > 0.1f)
    {
        color = texture(DiffuseTextureSampler, vec2(fragTexCoords.x, fragTexCoords.y)) * vec4(1.0f,1.0f,0.0f,1.0f);
    }
}
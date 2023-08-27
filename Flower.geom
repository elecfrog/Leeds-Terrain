#version 330 core
layout( points ) in; // Get Single Point In
layout( triangle_strip, max_vertices = 4 ) out; // Gen A Quard with 2 Triangles for Texture Rendering!

in VERT_DATA // Vertex Data From Vertex Shader, incluing heihgtRadio, and vertPostion in the worldSpace
{
    vec2 heihgtRadio;

	vec4 MVP_Position;
}vertDataIn[];

out vec2 fragHeihgtRadio;
out vec2 fragTexCoords;		// Simply Output UV coordinates for new triangles texture rendering

void main()
{
	// Calculate the screen position of the marker using ViewProjection matrix
	vec4 center = vertDataIn[0].MVP_Position;
	
	// Get the half length of the edge of the billboard so it can be drawn
	// from the center.
	vec2 size = vec2(2.0f, 2.0f);
	vec2 dir = size * 0.5;
	
	// [ ][x]
	// [ ][ ]
	gl_Position = vec4( center.x + dir.x, center.y+dir.y, center.z, center.w );
	fragTexCoords= vec2(1,0);
	fragHeihgtRadio = vertDataIn[0].heihgtRadio;
	EmitVertex();
	
	// [x][ ]
	// [ ][ ]
	gl_Position = vec4( center.x-dir.x, center.y+dir.y, center.z, center.w );
	fragTexCoords = vec2(0,0);
	fragHeihgtRadio = vertDataIn[0].heihgtRadio;
	EmitVertex();
	
	// [ ][ ]
	// [ ][x]
	gl_Position = vec4( center.x+dir.x, center.y-dir.y, center.z, center.w );
	fragTexCoords = vec2(1,1);
	fragHeihgtRadio = vertDataIn[0].heihgtRadio;
	EmitVertex();
	
	// [ ][ ]
	// [x][ ]
	gl_Position = vec4( center.x-dir.x, center.y-dir.y, center.z, center.w );
	fragTexCoords = vec2(0,1);
	fragHeihgtRadio = vertDataIn[0].heihgtRadio;
	EmitVertex();

	/// Finally emit the whole square.
	EndPrimitive();
}
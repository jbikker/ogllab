#version 330 core 
 
uniform vec2 nearfar;
uniform vec4 lightPos;

in vec4 wPos;

void main()
{
	// store radial depth, scaled by light 'far'
	gl_FragDepth = distance( wPos, lightPos ) / nearfar.y;
}

// EOF
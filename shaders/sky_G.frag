#version 330

in vec3 uv;
in vec3 wPos;

layout (location = 0) out vec3 worldOut;
layout (location = 1) out vec3 diffuseOut;
layout (location = 2) out vec3 normalOut;
layout (location = 3) out vec3 uvOut;

uniform samplerCube environment;

void main()
{
   normalOut = normalize( -uv );
   diffuseOut = texture( environment, uv ).rgb;
   worldOut = wPos;
   uvOut = normalOut;
}

// EOF
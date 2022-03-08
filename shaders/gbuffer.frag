#version 330

uniform sampler2D positions;
uniform sampler2D colors;
uniform sampler2D normals;
uniform sampler2D texCoords;

uniform vec3 eye;
uniform vec2 screenSize;

out vec3 pixel;

void main()
{
   vec2 uv = gl_FragCoord.xy / screenSize;
   vec3 position = texture( positions, uv ).xyz;
   vec3 diffuse = texture( colors, uv ).xyz;
   vec3 normal = texture( normals, uv ).xyz;
   vec3 texCoord = texture( texCoords, uv ).xyz;
   pixel = diffuse;
}

// EOF
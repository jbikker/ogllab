#include "precomp.h"

#define SPEEDSCALE 5.0f

extern Renderer renderer;

vec3 position;
float rotationY, rotationX, r = 0;

void Init()
{
	position = vec3( 0.0f, 0.0f, 0.0f ), rotationY = rotationX = 0;
	FILE* f = fopen( "camera.txt", "rb" );
	if (!f) return;
	vec3 forward, position;
	fread( &forward, 1, sizeof( vec3 ), f );
	fread( &position, 1, sizeof( vec3 ), f );
	Renderer::camera->SetForward( forward );
	Renderer::camera->SetPosition( position );
	fclose( f );
}

void HandleInput( float _Time )
{
	float speed = 0.0022f * _Time, mspeed = speed * 0.3f * SPEEDSCALE;
	if (GetAsyncKeyState( VK_LCONTROL )) mspeed *= 4.0f;
	vec3 left = Renderer::camera->GetLeft();
	vec3 up = Renderer::camera->GetUp();
	vec3 forward = Renderer::camera->GetForward();
	vec3 position = Renderer::camera->GetPosition();
	if (GetAsyncKeyState( 'R' )) position += mspeed * up;
	if (GetAsyncKeyState( 'F' )) position -= mspeed * up;
	if (GetAsyncKeyState( 'A' )) position += mspeed * left;
	if (GetAsyncKeyState( 'D' )) position -= mspeed * left;
	if (GetAsyncKeyState( 'W' )) position += mspeed * forward;
	if (GetAsyncKeyState( 'S' )) position -= mspeed * forward;
	Renderer::camera->SetPosition( position );
	if (GetAsyncKeyState( VK_UP )) forward = normalize( forward - up * speed );
	if (GetAsyncKeyState( VK_DOWN )) forward = normalize( forward + up * speed );
	if (GetAsyncKeyState( VK_LEFT )) forward = normalize( forward + left * speed );
	if (GetAsyncKeyState( VK_RIGHT )) forward = normalize( forward - left * speed );
	Renderer::camera->SetForward( forward );
}

void Tick( float _Time )
{
	HandleInput( _Time );
	PointLight* light = Renderer::scene->GetLight( 0 );
	r += 0.001f * _Time;
	if (r > 2 * PI) r -= 2 * PI;
	vec3 P = vec3( 3, 4, 12.5f ) + vec3( 0, 2 * sinf( r ), 0 );
	light->position = P;
	light->node->SetTransform( translate( mat4( 1 ), P ) );
	light = Renderer::scene->GetLight( 1 );
	P = vec3( -2, 4, -10 ) + vec3( 3 * sinf( r ), 0, 0 );
	light->position = P;
	light->node->SetTransform( translate( mat4( 1 ), P ) );
}

void Shutdown()
{
	FILE* f = fopen( "camera.txt", "wb" );
	vec3 forward = Renderer::camera->GetForward();
	vec3 position = Renderer::camera->GetPosition();
	fwrite( &forward, 1, sizeof( vec3 ), f );
	fwrite( &position, 1, sizeof( vec3 ), f );
	fclose( f );
}

// EOF
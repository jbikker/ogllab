#include "precomp.h"

Renderer renderer;
bool running = true;
HINSTANCE hInstance;

double timer::inv_freq = 1;

void CheckGL()
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		char t[1024];
		sprintf( t, "error %i (%x)\n", error, error );
		if (error == 0x500) strcat( t, "INVALID ENUM" );
		else if (error == 0x502) strcat( t, "INVALID OPERATION" );
		else if (error == 0x501) strcat( t, "INVALID VALUE" );
		else if (error == 0x506) strcat( t, "INVALID FRAMEBUFFER OPERATION" );
		else strcat( t, "UNKNOWN ERROR" );
		MessageBox( NULL, t, "OpenGL error", MB_OK | MB_ICONEXCLAMATION );
		exit( 0 );
	}
}

void CheckFrameBuffer()
{
	if (glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE)
	{
		MessageBox( NULL, "Incomplete frame buffer", "OpenGL error", MB_OK | MB_ICONEXCLAMATION );
		exit( 0 );
	}
}

void SaveBadShader( const char* _File, const char* _Text )
{
	uint line = 1;
	FILE* f = fopen( _File, "w" );
	char* pos = (char*)_Text, l[8];
	sprintf( l, "%03u  ", line );
	fwrite( l, 1, 5, f );
	while (*pos)
	{
		fwrite( pos, 1, 1, f );
		if (*pos++ != '\n') continue;
		sprintf( l, "%03u  ", ++line );
		fwrite( l, 1, 5, f );
	}
	fclose( f );
}

void SaveBadShaders( const char* _VShader, const char* _FShader, const char* _GShader )
{
	SaveBadShader( "badshader.vert", _VShader );
	SaveBadShader( "badshader.frag", _FShader );
	if (_GShader) SaveBadShader( "badshader.geom", _GShader );
}

void CheckShader( GLuint shader, const char* _VShader, const char* _FShader, const char* _GShader ) 
{
	char buffer[1024];
	memset( buffer, 0, 1024 );
	GLsizei length = 0;
	glGetShaderInfoLog( shader, 1024, &length, buffer );
	if (length > 0) if (strstr( buffer, "ERROR" ))
	{
		MessageBox( NULL, buffer, "Shader compile error", MB_OK | MB_ICONEXCLAMATION );
		SaveBadShaders( _VShader, _FShader, _GShader );
		exit( 0 );
	}
}

void CheckProgram( GLuint _ID, const char* _VShader, const char* _FShader, const char* _GShader ) 
{
	char buffer[1024];
	memset( buffer, 0, 1024 );
	GLsizei length = 0;
	glGetProgramInfoLog( _ID, 1024, &length, buffer );
	if (length > 0) 
	{
		MessageBox( NULL, buffer, "Shader link error", MB_OK | MB_ICONEXCLAMATION );
		SaveBadShaders( _VShader, _FShader, _GShader );
		exit( 0 );
	}
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) 
{
	switch (message) 
	{
	case WM_SIZE:
		renderer.ReshapeWindow( LOWORD( lParam ), HIWORD( lParam ) );
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) running = false;
		renderer.KeyDown( wParam & 255 );
		break;
	case WM_KEYUP:
		renderer.KeyUp( wParam & 255 );
		break;
	}
	return DefWindowProc( hWnd, message, wParam, lParam );
}

void RedirectIOToConsole() // from http://dslweb.nwnexus.com/~ast/dload/guicon.htm
{
	int hConHandle;
	HANDLE lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE* fp;
	AllocConsole();
	GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &coninfo );
	coninfo.dwSize.Y = 500;
	SetConsoleScreenBufferSize( GetStdHandle( STD_OUTPUT_HANDLE ), coninfo.dwSize );
	lStdHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
	lStdHandle = GetStdHandle( STD_INPUT_HANDLE );
	hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );
	lStdHandle = GetStdHandle( STD_ERROR_HANDLE );
	hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
	ios::sync_with_stdio();
}

bool createWindow( const char* title, int width, int height ) 
{
	WNDCLASS windowClass;
	HWND hWnd;
	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	hInstance = GetModuleHandle( NULL );
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = (WNDPROC)WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon( NULL, IDI_WINLOGO );
	windowClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = title;
	if (!RegisterClass( &windowClass )) return false;
	hWnd = CreateWindowEx( dwExStyle, title, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, width, height, NULL, NULL, hInstance, NULL );
	renderer.Init( hWnd );
	ShowWindow( hWnd, SW_SHOW );
	UpdateWindow( hWnd );
	return true;
}

void Tick( float _Time );
void Init();
void Shutdown();
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) 
{
	MSG msg;
	RedirectIOToConsole();
	createWindow( "OpenGL Laboratory", DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT );
	renderer.SetupScene();
	Init();
	LARGE_INTEGER f; 
	QueryPerformanceFrequency( &f ); 
	double inv_freq = 1000.0 / double( f.QuadPart );
	float frameTime = 0;
	while (running)
	{
		if (PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) 
		{
			if (msg.message == WM_QUIT) running = false; else 
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		else 
		{
			QueryPerformanceCounter( &f ); 
			long long start = f.QuadPart; 
			Tick( frameTime );
			renderer.Render();
			QueryPerformanceCounter( &f ); 
			frameTime = (float)((f.QuadPart - start) * inv_freq);
		}
	}
	Shutdown();
	return (int)msg.wParam;
}

// EOF
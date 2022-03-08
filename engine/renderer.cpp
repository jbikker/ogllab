#include "precomp.h"

UniformBuffer::UniformBuffer( void* _Data, uint _Size, uint _Point ) : data( (float*)_Data ), size( _Size ), bindingPoint( _Point )
{
	glGenBuffers( 1, &id );
}

void UniformBuffer::Update()
{
	glBindBuffer( GL_UNIFORM_BUFFER, id );
	glBufferData( GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW );
	glBindBufferBase( GL_UNIFORM_BUFFER, bindingPoint, id );
}

TextureBuffer::TextureBuffer( uint _Size ) : size( _Size )
{
	data = new float[size];
	glGenBuffers( 1, &tbo );
	glGenTextures( 1, &tex );
	glBindTexture( GL_TEXTURE_BUFFER, 0 );
	CheckGL();
}

void TextureBuffer::Update() 
{
	glBindBuffer( GL_TEXTURE_BUFFER, tbo );
	glBufferData( GL_TEXTURE_BUFFER, size * sizeof( float ), data, GL_STATIC_DRAW );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_BUFFER, tex );
	glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, tbo );
	glBindTexture( GL_TEXTURE_BUFFER, 0 );
	CheckGL();
}

Renderer::Renderer()
{
	key = new bool[256];
	memset( &statics, 0, sizeof( statics ) );
}

Renderer::~Renderer() 
{
    wglMakeCurrent( hdc, 0 );	// remove the rendering context from our device context
    wglDeleteContext( hrc );	// delete rendering context
    ReleaseDC( hwnd, hdc );		// release device context from our window
	delete key;
}

void Renderer::Init( HWND _Handle )
{
	hwnd = _Handle;
	hdc = GetDC( hwnd );
	Create30Context();
	// uniform buffers
	statics.lightUBO = new UniformBuffer( statics.lightUniforms, sizeof( statics.lightUniforms ), 0 );
	statics.transformUBO = new UniformBuffer( statics.transformUniforms, sizeof( statics.transformUniforms ), 1 );
	statics.commonUBO = new UniformBuffer( statics.commonUniforms, sizeof( statics.commonUniforms ), 2 );
	// technique
#if (TECHNIQUE == HQTECHNIQUE)
	technique = new HQTechnique();
#elif (TECHNIQUE == BASICTECHNIQUE)
	technique = new BasicTechnique();
#else // TECHNIQUE = STOCHASTICTECHNIQUE
	technique = new StochasticTechnique();
#endif
	technique->Init();
}

bool Renderer::Create30Context() 
{
	PIXELFORMATDESCRIPTOR pfd;
	memset( &pfd, 0, sizeof( PIXELFORMATDESCRIPTOR ) );
	pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int nPixelFormat = ChoosePixelFormat( hdc, &pfd );
	if (nPixelFormat == 0) return false;
	int bResult = SetPixelFormat( hdc, nPixelFormat, &pfd );
	if (!bResult) return false;
	HGLRC tempOpenGLContext = wglCreateContext( hdc );
	wglMakeCurrent( hdc, tempOpenGLContext );
	GLenum error = glewInit();
	if (error != GLEW_OK) return false;
	int attributes[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, 0
	};
	if (wglewIsSupported( "WGL_ARB_create_context" ) == 1) 
	{
		hrc = wglCreateContextAttribsARB( hdc, NULL, attributes );
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( tempOpenGLContext );	// delete temporary OpenGL 2.1 context
		wglMakeCurrent( hdc, hrc );				// make 3.0 context current
	}
	else hrc = tempOpenGLContext;				// no support for OpenGL 3.x and up, use 2.1
	printf( "device: %s\n", glGetString( GL_RENDERER ) );
	return true;
}

void Renderer::SetupScene() 
{
	scene = new Scene();
	camera = new Camera();
	glClearColor( 0, 0, 0, 0 );
	Pass::SetScene( scene );
}

void Renderer::ReshapeWindow( uint w, uint h ) 
{
	technique->Resize( w, h );
}

void Renderer::Render()
{
	scene->UpdateLights();
	// sync uniform data
	projMatrix = perspective( PI / 4.1f, technique->GetScreenBuffer()->GetAspectRatio(), 0.01f, 500.0f );
	viewMatrix = projMatrix * camera->GetViewMatrix();
	vec2 screenSize = technique->GetScreenBuffer()->GetSize();
	statics.screenWidth = screenSize.x, statics.screenHeight = screenSize.y;
	statics.eye = camera->GetPosition();
	statics.cubeNear = 0.1f;
	statics.cubeFar = 200.0f;
	statics.lightUBO->Update();
	statics.commonUBO->Update();
	// render
	technique->Render();
	// present
	SwapBuffers( hdc );
	CheckGL();
}

// EOF
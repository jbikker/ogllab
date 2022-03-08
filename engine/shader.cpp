#include "precomp.h"

vector<Shader*> Shader::cache;

static string textFileRead( const char* _File ) 
{
	string data, line;
	ifstream f( _File );
	if (f.is_open()) 
	{ 
		while (!f.eof()) 
		{
			getline( f, line );
			data.append( line );
			data.append( "\n" );
		}
		f.close();
	}
	return data;
}

#define FOR_ALL(x) code.append( x ); code.append( "\n" );
#define FOR_ALL_(x,y,z) code.append( x ); code.append( y ); code.append( z ); code.append( "\n" );
#define FOR_TEXT(x) if (_Flags & TEXTURE) { code.append( x ); code.append( "\n" ); }
#define FOR_NOTX(x) if (!(_Flags & TEXTURE)) { code.append( x ); code.append( "\n" ); }
#define FOR_NMAP(x) if (_Flags & NORMALMAP) { code.append( x ); code.append( "\n" ); }
#define FOR_NONM(x) if (!(_Flags & NORMALMAP)) { code.append( x ); code.append( "\n" ); }
#define FOR_ENVM(x) if (_Flags & ENVMAP) { code.append( x ); code.append( "\n" ); }
#define FOR_SHDW(x) if (_Flags & SHADOW) { code.append( x ); code.append( "\n" ); }
#define FOR_CUBE(x) if (_Flags & SHADOWCUBE) { code.append( x ); code.append( "\n" ); }
#define FOR_CUB_(x,y,z) if (_Flags & SHADOWCUBE) { code.append( x ); code.append( y ); code.append( z ); code.append( "\n" ); }
#define FOR_NOSH(x) if (!(_Flags & SHADOW)) { code.append( x ); code.append( "\n" ); }
#define FOR_GEOM(x) if (_Flags & GEOMETRY) { code.append( x ); code.append( "\n" ); }
#define FOR_NOGB(x) if (!(_Flags & GEOMETRY)) { code.append( x ); code.append( "\n" ); }
#define FOR_WRLD(x) if (_Flags & (GEOMETRY | ENVMAP)) { code.append( x ); code.append( "\n" ); }
#define FOR_CMSP(x) if (!(_Flags & (GEOMETRY | ENVMAP))) { code.append( x ); code.append( "\n" ); }
#define FOR_ALPH(x) if (_Flags & ALPHA) { code.append( x ); code.append( "\n" ); }

Shader::Shader( ShaderFlags _Flags ) : name( 0 )
{
	string code, vcode, fcode;
	char ls[16], dyn[16];
	sprintf( ls, "%i", MAXLIGHTS );
	sprintf( dyn, "%i", MAXDYNAMIC );
	// generate vertex shader code
	FOR_ALL(  "#version 330 core" );
	FOR_ALL(  "" );
	FOR_ALL(  "layout(location = 0) in vec4 pos;" );
	FOR_ALL(  "layout(location = 1) in vec3 vn;" );
	FOR_TEXT( "layout(location = 2) in vec2 vuv;" );
	FOR_ALL(  "" );
	FOR_ALL(  "layout(std140) uniform TransformBlock" );
	FOR_ALL(  "{" );
	FOR_ALL(  "   mat4 view;" );
	FOR_ALL(  "   mat4 world;" );
	FOR_ALL(  "};" );
	FOR_ALL(  "" );
	FOR_SHDW( "uniform mat4 light;" );
	FOR_ALL(  "" );
	FOR_ALL(  "out vec3 N;" );
	FOR_TEXT( "out vec2 uv;" );
	FOR_ALL(  "out vec3 wPos;" );
	FOR_SHDW( "out vec4 shadowPos;" );
	FOR_ALL(  "" );
	FOR_ALL(  "void main()" );
	FOR_ALL(  "{" );
	FOR_ALL(  "   gl_Position = view * pos;" );
	FOR_ALL(  "   wPos = (world * pos).xyz;" );
	FOR_SHDW( "   shadowPos = light * pos;" );
	FOR_TEXT( "   uv = vuv;" );
	FOR_ALL(  "   N = (world * vec4( vn, 0 )).xyz;" );
	FOR_ALL(  "}" );
	FOR_ALL(  "\n// EOF" );
	vcode = code;
	code.clear();
	// generate fragment shader code
	if (_Flags & GEOMETRY)
	{
		// geometry shaders
		FOR_ALL(  "#version 330 core" );
		FOR_ALL(  "" );
		FOR_ALL(  "layout(std140) uniform CommonBlock" );
		FOR_ALL(  "{" );
		FOR_ALL(  "   vec3 eye;" );
		FOR_ALL(  "   int dummy;" );
		FOR_ALL(  "   vec2 screenSize;" );
		FOR_ALL(  "   vec2 cubeNearFar;" );
		FOR_ALL(  "};" );
		FOR_ALL(  "" );
		FOR_ALL(  "in vec3 N;" );
		FOR_TEXT( "in vec2 uv;" );
		FOR_ALL(  "in vec3 wPos;" );
		FOR_ALL(  "" );
		FOR_ALL(  "layout (location = 0) out vec3 worldOut;" );
		FOR_ALL(  "layout (location = 1) out vec3 diffuseOut;" );
		FOR_ALL(  "layout (location = 2) out vec3 normalOut;" );
		FOR_ALL(  "" );
		FOR_NOTX( "uniform vec3 color;" );
		FOR_NOTX(  "" );
		FOR_TEXT( "uniform sampler2D sampler;" );
		FOR_NMAP( "uniform sampler2D normals;" );
		FOR_ALL(  "" );
		if (_Flags & NORMALMAP) IncludeCode( code, "shaders/perturb.func" );
		FOR_ALL(  "" );
		FOR_ALL(  "void main()" );
		FOR_ALL(  "{" );
		FOR_TEXT( "   vec4 diffuse = texture( sampler, uv );" );
		FOR_NOTX( "   vec4 diffuse = vec4( color, 1 );" );
		FOR_ALPH( "   if (diffuse.a < 0.5) discard; else diffuse.a = 1.0f;" );
		FOR_NONM( "   normalOut = normalize( N );" );
		FOR_NMAP( "   vec3 V = normalize( eye - wPos );" );
		FOR_NMAP( "   normalOut = perturb_normal( N, -V, uv );" );
		FOR_ALL(  "   diffuseOut = diffuse.rgb;" );
		FOR_ALL(  "   worldOut = wPos;" );
		FOR_ALL(  "}" );
		FOR_ALL(  "\n// EOF" );
	}
	else
	{
		// shadow & direct shaders
		FOR_ALL(  "#version 330 core" );
		FOR_ALL(  "" );
		FOR_ALL(  "in vec3 N;" );
		FOR_TEXT( "in vec2 uv;" );
		FOR_SHDW( "in vec4 shadowPos;" );
		FOR_ALL(  "in vec3 wPos;" );
		FOR_ALL(  "" );
		FOR_ALL(  "out vec4 pixel;" );
		FOR_ALL(  "" );
		FOR_ALL(  "struct PointLight" );
		FOR_ALL(  "{" );
		FOR_ALL(  "   vec4 color;" );
		FOR_ALL(  "   vec4 position;" );
		FOR_ALL(  "};" );
		FOR_ALL(  "" );
		FOR_ALL(  "layout(std140) uniform CommonBlock" );
		FOR_ALL(  "{" );
		FOR_ALL(  "   vec3 eye;" );
		FOR_ALL(  "   int dummy;" );
		FOR_ALL(  "   vec2 screenSize;" );
		FOR_ALL(  "   vec2 cubeNearFar;" );
		FOR_ALL(  "};" );
		FOR_ALL(  "" );
		FOR_ALL(  "layout(std140) uniform LightsBlock" );
		FOR_ALL(  "{" );
		FOR_ALL_( "   PointLight pointLight[", ls, "];" );
		FOR_ALL(  "   int lightCount;" );
		FOR_ALL(  "};" );
		FOR_ALL(  "" );
		FOR_NOTX( "uniform vec3 color;" );
		FOR_ALL(  "uniform sampler2D AO;" );
		FOR_CUB_( "uniform samplerCubeShadow shadowCube[", dyn, "];" );
		FOR_SHDW( "uniform sampler2DShadow shadowMap;" );
		FOR_TEXT( "uniform sampler2D sampler;" );
		FOR_NMAP( "uniform sampler2D normals;" );
		FOR_ENVM( "uniform samplerCube environment;" );
		FOR_ALL(  "" );
		IncludeCode( code, "shaders/shade.func" );
		FOR_ALL(  "" );
		if (_Flags & NORMALMAP) IncludeCode( code, "shaders/perturb.func" );
		FOR_NMAP( "" );
	#if 0
		FOR_ALL(  "void main()" );
		FOR_ALL(  "{" );
		FOR_ALL(  "   float ao = texture( AO, vec2( gl_FragCoord.xy / screenSize ) );" );
		FOR_ALL(  "   pixel = vec4( ao, ao, ao, 1 );" );
		FOR_ALL(  "}" );
	#else
		FOR_ALL(  "void main()" );
		FOR_ALL(  "{" );
		FOR_TEXT( "   vec4 diffuse = texture( sampler, uv );" );
		FOR_NOTX( "   vec4 diffuse = vec4( color, 1 );" );
		FOR_ALPH( "   if (diffuse.a < 0.5) discard; else diffuse.a = 1.0f;" );
		FOR_ALL(  "   vec3 V = normalize( eye - wPos );" );
		FOR_NONM( "   vec3 Nc = normalize( N );" );
		FOR_NMAP( "   vec3 Nc = perturb_normal( N, -V, uv );" );
		FOR_ALL(  "   float ao = texelFetch( AO, ivec2( gl_FragCoord.xy ), 0 ).r;" );
		FOR_ALL(  "   vec3 c = vec3( 0 );" );
		FOR_ALL(  "   for( int i = 0; i < lightCount; i++ )" );
		FOR_ALL(  "   {" );
		FOR_ALL(  "      vec3 L = pointLight[i].position.xyz - wPos;" );
		FOR_ALL(  "      float visibility = 1;" );
		FOR_SHDW( "      visibility = texture( shadowMap, vec3( shadowPos.xy, (shadowPos.z - 0.005) / shadowPos.w ) );" );
		FOR_CUB_( "      if (i < ", dyn, ")" );
		FOR_CUBE( "      {" );
		FOR_CUBE( "         float fragDepth = length( L );" );
		FOR_CUBE( "         float bias = 0.03 * tan( acos( dot( N, normalize( L ) ) ) );" );
		FOR_CUBE( "         visibility = texture( shadowCube[i], vec4( -L, (fragDepth - bias) / cubeNearFar.y ) );" );
		FOR_CUBE( "      }" );
		FOR_ALL(  "      c += Shade( diffuse.rgb, pointLight[i].color.rgb, Nc, L, V, visibility );" );
		FOR_ALL(  "   }" );
		FOR_ALL(  "   c += vec3( 0.05, 0.05, 0.04 ) * diffuse.rgb;" );
		FOR_ALL(  "   c *= ao;" );
		FOR_ENVM( "   float refl = 0.8;" );
		FOR_ENVM( "   vec3 R = reflect( normalize( wPos - eye), Nc );" );
		FOR_ENVM( "   c = c * (1.0 - refl) + refl * (diffuse * texture( environment, R )).rgb;" );
		FOR_ALL(  "   pixel = vec4( c, (N.y > 0.99) ? 0 : 1 );" );
		FOR_ALL(  "}" );
	#endif
		FOR_ALL(  "\n// EOF" );
	}
	fcode = code;
	// compile
	char name[256];
	ConstructName( _Flags, name );
	SetName( name );
#if 1
	char vs[256], fs[256];
	strcpy( vs, "shaders/debug/" );
	strcat( vs, name );
	strcat( vs, ".vert" );
	strcpy( fs, "shaders/debug/" );
	strcat( fs, name );
	strcat( fs, ".frag" );
	FILE* test = fopen( vs, "w" );
	const char* vt = vcode.c_str(), *ft = fcode.c_str();
	fwrite( vt, 1, strlen( vt ) + 1, test );
	fclose( test );
	test = fopen( fs, "w" );
	fwrite( ft, 1, strlen( ft ) + 1, test );
	fclose( test );
#endif
	Compile( vcode.c_str(), fcode.c_str() );
	cache.push_back( this );
	// link uniform blocks to binding points
	if (!(_Flags & GEOMETRY)) glUniformBlockBinding( id, glGetUniformBlockIndex( id, "LightsBlock" ), 0 );
	glUniformBlockBinding( id, glGetUniformBlockIndex( id, "TransformBlock" ), 1 );
	glUniformBlockBinding( id, glGetUniformBlockIndex( id, "CommonBlock" ), 2 );
	// texture bindings
	glUseProgram( id );
	uint idx = 0;
	if (!(_Flags & GEOMETRY))
	{
		glUniform1i( glGetUniformLocation( id, "AO" ), idx++ );
		if (_Flags & SHADOWCUBE) for( int i = 0; i < MAXDYNAMIC; i++ )
		{
			char name[] = "shadowCube[0]";
			name[11] = '0' + i;
			glUniform1i( glGetUniformLocation( id, name ), idx++ );
		}
		if (_Flags & SHADOW) glUniform1i( glGetUniformLocation( id, "shadowMap" ), idx++ );
	}
	if (_Flags & TEXTURE) glUniform1i( glGetUniformLocation( id, "sampler" ), idx++ );
	if (_Flags & NORMALMAP) glUniform1i( glGetUniformLocation( id, "normals" ), idx++ );
	if (!(_Flags & GEOMETRY)) if (_Flags & ENVMAP) glUniform1i( glGetUniformLocation( id, "environment" ), idx++ );
	glUseProgram( 0 );
	CheckGL();
}

Shader::Shader( const char* _VFile, const char* _PFile ) : name( 0 )
{
    Init( _VFile, _PFile );
}

Shader::Shader( const char* _GFile, const char* _VFile, const char* _PFile ) : name( 0 )
{
	Init( _VFile, _PFile, _GFile );
}

Shader* Shader::GetCached( ShaderFlags _Flags )
{
	char name[256];
	ConstructName( _Flags, name );
	for( size_t s = cache.size(), i = 0; i < s; i++ )
		if (!strcmp( cache[i]->GetName(), name )) return cache[i];
	return 0;
}

void Shader::IncludeCode( string& _Code, const char* _File )
{
	FILE* f = fopen( _File, "r" );
	if (!f) return;
	char buffer[1024];
	while (!feof( f ))
	{
		buffer[0] = 0;
		fgets( buffer, 1022, f );
		while ((buffer[0]) && (buffer[strlen( buffer ) - 1] < 33)) buffer[strlen( buffer ) - 1] = 0;
		_Code.append( buffer );
		_Code.append( "\n" );
	}
}

void Shader::ConstructName( ShaderFlags _Flags, char* _Name )
{
	char name[] = "shader_______";
	if (_Flags & TEXTURE) name[7] = 'T';
	if (_Flags & NORMALMAP) name[8] = 'N';
	if (_Flags & ALPHA) name[9] = 'A';
	if (_Flags & ENVMAP) name[10] = 'E';
	if (_Flags & SHADOW) name[11] = 'S';
	if (_Flags & SHADOWCUBE) name[11] = 'C';
	if (_Flags & GEOMETRY) name[12] = 'G';
	strcpy( _Name, name );
}

void Shader::SetName( const char* _Name )
{
	delete name;
	name = new char[strlen( _Name ) + 1];
	strcpy( name, _Name );
}

void Shader::Init( const char* _VFile, const char* _PFile, const char* _GFile ) 
{
	char gfile[1024], vfile[1024], pfile[1024];
	strcpy( vfile, "shaders/" );
	strcpy( pfile, "shaders/" );
	strcat( vfile, _VFile );
	strcat( pfile, _PFile );
	string vsText = textFileRead( vfile );
	string fsText = textFileRead( pfile ), gsText;
	const char* vertexText = vsText.c_str();
	const char* fragmentText = fsText.c_str();
	const char* geometryText = 0;
	if (_GFile)
	{
		strcpy( gfile, "shaders/" );
		strcat( gfile, _GFile );
		gsText = textFileRead( gfile );
		geometryText = gsText.c_str();
	}
	Compile( vertexText, fragmentText, geometryText );
}

void Shader::Compile( const char* _Vertex, const char* _Fragment, const char* _Geometry )
{
	vertex = glCreateShader( GL_VERTEX_SHADER );
	pixel = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( vertex, 1, &_Vertex, 0 );
	glCompileShader( vertex );
	CheckShader( vertex, _Vertex, _Fragment, _Geometry );
	glShaderSource( pixel, 1, &_Fragment, 0 );
	glCompileShader( pixel );
	CheckShader( pixel, _Vertex, _Fragment, _Geometry );
	id = glCreateProgram();
	glAttachShader( id, vertex );
	glAttachShader( id, pixel );
	if (_Geometry)
	{
		geometry = glCreateShader( GL_GEOMETRY_SHADER );
		glShaderSource( geometry, 1, &_Geometry, 0 );
		glCompileShader( geometry );
		CheckShader( geometry, _Vertex, _Fragment, _Geometry );
		glAttachShader( id, geometry );
	}
	glBindAttribLocation( id, 0, "in_Position" );
	glBindAttribLocation( id, 1, "in_Color" );
	glLinkProgram( id );
	CheckProgram( id, _Vertex, _Fragment, _Geometry );
}

Shader::~Shader() 
{
	glDetachShader( id, pixel );
	glDetachShader( id, vertex );
	glDeleteShader( pixel );
	glDeleteShader( vertex );
	glDeleteProgram( id );
}

uint Shader::GetID() 
{
    return id;
}

void Shader::Bind() 
{ 
    glUseProgram( id );
}

void Shader::Unbind() 
{
    glUseProgram( 0 );
}

// EOF
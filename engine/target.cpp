#include "precomp.h"

void RenderTarget::Resize( uint _Width, uint _Height )
{
	width = _Width;
	height = _Height;
}

void RenderTarget::Unbind()
{
	glUseProgram( 0 );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
}

ShadowCubeTarget::ShadowCubeTarget( uint _Width, uint _Height )
{
	Resize( _Width, _Height );
	glGenTextures( 1, &cubeMapID );
	glBindTexture( GL_TEXTURE_CUBE_MAP, cubeMapID );
	for( int i = 0; i < 6; i++ )
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glGenFramebuffers( 1, &fbo );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo );
	glFramebufferTexture( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeMapID, 0 );
	glDrawBuffer( GL_NONE );
	glReadBuffer( GL_NONE );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
}

void ShadowCubeTarget::Bind( int _Face, vec3 _LightPos )
{
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo );
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + _Face, cubeMapID, 0 );
	glDrawBuffer( GL_NONE );
	glReadBuffer( GL_NONE );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
#if 1
	glEnable( GL_CULL_FACE );
	glCullFace( GL_FRONT );
#else
	glDisable( GL_CULL_FACE );
#endif
	glViewport( 0, 0, width, height );
	glClear( GL_DEPTH_BUFFER_BIT );
}

CubemapTarget::CubemapTarget( uint _Width, uint _Height )
{
	// store width and height
	Resize( _Width, _Height );
	// generate cubemap
	glGenTextures( 1, &cubeMapID );
	glBindTexture( GL_TEXTURE_CUBE_MAP, cubeMapID );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	for( int i = 0; i < 6; i++ )
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
	// generate depth map
	glGenTextures( 1, &depthMapID );
	glBindTexture( GL_TEXTURE_2D, depthMapID );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
	glBindTexture( GL_TEXTURE_2D, 0 );
	// construct fbo
	glGenFramebuffers( 1, &fbo );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
}

void CubemapTarget::Bind( uint _Idx )
{
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo );
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + _Idx, cubeMapID, 0 );
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapID, 0 );
	glDrawBuffer( GL_COLOR_ATTACHMENT0 );
	glViewport( 0, 0, width, height );
	glClear( GL_DEPTH_BUFFER_BIT );
}

GeometryBuffer::GeometryBuffer( uint _Width, uint _Height )
{
	width = _Width;
	height = _Height;
	glGenFramebuffers( 1, &fbo );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo );
	memset( buffer, 0, 4 * sizeof( int ) );
	glGenTextures( 3, buffer );
    glGenTextures( 1, &depth );
	for( int i = 0; i < 3; i++ )
	{
		glBindTexture( GL_TEXTURE_2D, buffer[i] );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, buffer[i], 0 );
	}
	glBindTexture( GL_TEXTURE_2D, depth );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0 );
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 }; 
	glDrawBuffers( 3, buffers );
	CheckFrameBuffer();
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
}

void GeometryBuffer::Bind()
{
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void GeometryBuffer::Resize( uint _Width, uint _Height )
{
	width = _Width;
	height = _Height;
	for( int i = 0; i < 3; i++ )
	{
		glBindTexture( GL_TEXTURE_2D, buffer[i] );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL );
	}
	glBindTexture( GL_TEXTURE_2D, depth );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
} 

GenericTarget::GenericTarget( uint _Width, uint _Height, TargetFlags _Flags )
{
	width = _Width;
	height = _Height;
	flags = _Flags;
	// prepare fbo
	fbo = 0;
	glGenFramebuffers( 1, &fbo );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo );
	// color surface
	if (flags & TARGET_COLOR)
	{
		if (flags == TARGET_1CHANNEL)
		{
			colorFormat = GL_R32F;
			colorLayout = GL_RED;
			colorType = GL_FLOAT;
		}
		else if (flags & TARGET_FLOATCOLOR) 
		{
			colorFormat = GL_RGBA32F;
			colorLayout = GL_RGBA;
			colorType = GL_FLOAT;
		}
		else 
		{
			colorFormat = GL_RGBA8;
			colorType = GL_UNSIGNED_BYTE;
		}
		colorID = 0;
		glGenTextures( 1, &colorID );
		glBindTexture( GL_TEXTURE_2D, colorID );
		glTexImage2D( GL_TEXTURE_2D, 0, colorFormat, width, height, 0, colorLayout, colorType, 0 );
		if (flags & TARGET_MIPMAP)
		{
			int w = width, h = height;
			for( int i = 1; i <= 5; i++ )
			{
				w >>= 1, h >>= 1;
				glTexImage2D( GL_TEXTURE_2D, i, colorFormat, w, h, 0, colorLayout, colorType, 0 );
			}
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5 );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		}
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
 		glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorID, 0 );
		glDrawBuffer( GL_COLOR_ATTACHMENT0 );
	}
	else
	{
		glDrawBuffer( GL_NONE );
	}
	// depth buffer
	if (flags & TARGET_DEPTH)
	{
		if (flags & TARGET_DEPTH16) depthFormat = GL_DEPTH_COMPONENT16, depthType = GL_FLOAT;
							   else depthFormat = GL_DEPTH_COMPONENT32F, depthType = GL_FLOAT;
		depthID = 0;
		glGenTextures( 1, &depthID );
		glBindTexture( GL_TEXTURE_2D, depthID );
		glTexImage2D( GL_TEXTURE_2D, 0, depthFormat, width, height, 0, GL_DEPTH_COMPONENT, depthType, 0 );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
 		glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthID, 0 );
	}
	else
	{
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 );
	}
	CheckFrameBuffer();
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
}

void GenericTarget::Resize( uint _Width, uint _Height )
{
	width = _Width;
	height = _Height;
	if (flags & TARGET_COLOR)
	{
		glBindTexture( GL_TEXTURE_2D, colorID );
		glTexImage2D( GL_TEXTURE_2D, 0, colorFormat, width, height, 0, colorLayout, colorType, 0 );
		if (flags & TARGET_MIPMAP)
		{
			uint w = width, h = height;
			for( int i = 1; i <= 5; i++ )
			{
				w >>= 1, h >>= 1;
				glTexImage2D( GL_TEXTURE_2D, i, colorFormat, w, h, 0, colorLayout, colorType, 0 );
			}
		}
	}
	if (flags & TARGET_DEPTH)
	{
		glBindTexture( GL_TEXTURE_2D, depthID );
		glTexImage2D( GL_TEXTURE_2D, 0, depthFormat, width, height, 0, GL_DEPTH_COMPONENT, depthType, 0 );
	}
}

void GenericTarget::Bind()
{
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo );
	glViewport( 0, 0, width, height );
	if (flags & TARGET_DEPTH)
	{
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_LESS );
	}
	else glDisable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	if (clear)
	{
	    glClear( ((flags & TARGET_COLOR) ? GL_COLOR_BUFFER_BIT : 0) | ((flags & TARGET_DEPTH) ? GL_DEPTH_BUFFER_BIT : 0) );
		clear = false;
	}
}

void GenericTarget::Clear()
{
	clear = true;
}

ScreenBuffer::ScreenBuffer( uint _Width, uint _Height )
{
	Resize( _Width, _Height );
	fbo = 0;
}

void ScreenBuffer::Bind()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glViewport( 0, 0, width, height );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
}

// EOF
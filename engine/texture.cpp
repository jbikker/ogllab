#include "precomp.h"

Texture::Texture() : name( 0 ), id( 0 ), hasAlpha( false )
{
}

Texture::Texture( const char* _File ) : name( 0 ), id( 0 ), hasAlpha( false )
{
	Load( _File );
	SetName( _File );
}

Texture::~Texture()
{
	delete name;
}

void Texture::SetName( const char* _Name )
{
	delete name;
	name = new char[strlen( _Name ) + 1];
	strcpy( name, _Name );
}

void Texture::sRGBtoLinear( unsigned char* _Data, uint _Size, uint _Stride )
{
	for( uint j = 0; j < _Size; j++ )
	{
		_Data[j * _Stride + 0] = (_Data[j * _Stride + 0] * _Data[j * _Stride + 0]) >> 8;
		_Data[j * _Stride + 1] = (_Data[j * _Stride + 1] * _Data[j * _Stride + 1]) >> 8;
		_Data[j * _Stride + 2] = (_Data[j * _Stride + 2] * _Data[j * _Stride + 2]) >> 8;
	}
}

void Texture::Load( const char* _File )
{
	bool loaded = false;
	GLuint textureType = GL_TEXTURE_2D;
	glGenTextures( 1, &id );
	glBindTexture( textureType, id );
	if (strstr( _File, ".tga" ) == (_File + strlen( _File ) - 4))
	{
		// special case: tga files; bypass FreeImage for speed
		FILE* tga = fopen( _File, "rb" );
		unsigned char header[18];
		fread( header, 1, 18, tga );
		uint width = header[12] + 256 * header[13], height = header[14] + 256 * header[15], type = header[2], bpp = header[16];
		if ((type == 2) && (bpp == 32))
		{
			unsigned char* data = new unsigned char[width * height * 4], *flipped = new unsigned char[width * height * 4];
			fread( data, 4, width * height, tga );
			for ( uint y = 0; y < height; y++ ) memcpy( flipped + y * width * 4, data + (height - y - 1) * width * 4, width * 4 );
			sRGBtoLinear( flipped, width * height, 4 );
			glTexImage2D( textureType, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, flipped );
			loaded = true;
			delete data;
			delete flipped;
		}
		fclose( tga );
	}
	if (!loaded)
	{
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType( _File, 0 );
		if (fif == FIF_UNKNOWN) fif = FreeImage_GetFIFFromFilename( _File );
		if (fif == FIF_HDR)
		{
			// load hdr image
			FIBITMAP* dib = FreeImage_Load( FIF_HDR, _File );
			uint width = FreeImage_GetWidth( dib ), height = FreeImage_GetHeight( dib );
			float* data = new float[width * height * 3];
			for( uint y = 0; y < height; y++) 
				memcpy( data + y * width * 3, (float*)FreeImage_GetScanLine( dib, height - 1 - y ), width * 12 );
			FreeImage_Unload( dib );
			glTexImage2D( textureType, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data );
			delete data;
		}
		else
		{
			// load ldr image
			FIBITMAP* tmp = FreeImage_Load( fif, _File ), *dib = FreeImage_ConvertTo24Bits( tmp );
			FreeImage_Unload( tmp );
			uint width = FreeImage_GetWidth( dib ), height = FreeImage_GetHeight( dib );
			unsigned char* data = new unsigned char[width * height * 3];
			for( uint y = 0; y < height; y++) 
				memcpy( data + y * width * 3, FreeImage_GetScanLine( dib, height - 1 - y ), width * 3 );
			FreeImage_Unload( dib );
			sRGBtoLinear( data, width * height, 3 );
			glTexImage2D( textureType, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data );
			delete data;
		}
	}
	glTexParameteri( textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glGenerateMipmap( textureType );
}

void Texture::LoadCubemap( const char** _Files )
{
	glGenTextures( 1, &id );
	GLuint textureType = GL_TEXTURE_CUBE_MAP;
	glBindTexture( textureType, id );
	for( int i = 0; i < 6; i++ )
	{
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType( _Files[i], 0 );
		if (fif == FIF_UNKNOWN) fif = FreeImage_GetFIFFromFilename( _Files[i] );
		if (fif == FIF_HDR)
		{
			// load hdr image
			FIBITMAP* dib = FreeImage_Load( FIF_HDR, _Files[i] );
			uint width = FreeImage_GetWidth( dib ), height = FreeImage_GetHeight( dib );
			float* data = new float[width * height * 3];
			for( uint y = 0; y < height; y++) 
				memcpy( data + y * width * 3, (float*)FreeImage_GetScanLine( dib, height - 1 - y ), width * 12 );
			FreeImage_Unload( dib );
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data );
			delete data;
		}
		else
		{
			// load ldr image
			FIBITMAP* tmp = FreeImage_Load( fif, _Files[i] ), *dib = FreeImage_ConvertTo24Bits( tmp );
			FreeImage_Unload( tmp );
			uint width = FreeImage_GetWidth( dib ), height = FreeImage_GetHeight( dib );
			unsigned char* data = new unsigned char[width * height * 3];
			for( uint y = 0; y < height; y++) 
				memcpy( data + y * width * 3, FreeImage_GetScanLine( dib, height - 1 - y ), width * 3 );
			FreeImage_Unload( dib );
			sRGBtoLinear( data, width * height, 3 );
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data );
			delete data;
		}
	}
	glTexParameteri( textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glGenerateMipmap( textureType );
	glBindTexture( textureType, 0 );
}

// EOF
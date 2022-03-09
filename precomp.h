#pragma once

// system includes
#include <GL/glew.h>
#include <GL/wglew.h>
#include <vector>
#include "common.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <fcntl.h>
#include <io.h>

// external libraries
#include "freeimage.h"
#include <string>

// opengl lib includes
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")

using namespace std;
using namespace glm;

// library includes
#include "maths.h"
#include "shader.h"
#include "system.h"
#include "light.h"
#include "texture.h"
#include "material.h"
#include "node.h"
#include "scene.h"
#include "camera.h"
#include "target.h"
#include "pass.h"
#include "technique.h"
#include "renderer.h"

// library namespace
using namespace ogllab;

// global helper functions
void CheckGL();
void CheckFrameBuffer();
void CheckShader( GLuint shader, const char* _VShader, const char* _FShader, const char* _GShader );
void SaveBadShaders( const char* _VShader, const char* _FShader, const char* _GShader );
void CheckProgram( GLuint _ID, const char* _VShader, const char* _FShader, const char* _GShader );

// EOF
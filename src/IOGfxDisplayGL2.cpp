/**
 * Graphics primitives with OpenGL (ES) 2

 * Copyright (C) 2015  Sylvain Beucler

 * This file is part of GNU FreeDink

 * GNU FreeDink is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.

 * GNU FreeDink is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public
 * License along with GNU FreeDink.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "IOGfxDisplayGL2.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string.h>

#include "log.h"
#include "IOGfxGLFuncs.h"
#include "IOGfxSurfaceGL2.h"
#include "gfx_palette.h"
#include "ImageLoader.h"


IOGfxDisplayGL2::IOGfxDisplayGL2(int w, int h, bool truecolor, Uint32 flags)
	: IOGfxDisplay(w, h, truecolor, flags | SDL_WINDOW_OPENGL),
	  glcontext(NULL), gl(NULL), screen(NULL) {
	palette = -1;
	vboSpriteVertices = -1, vboSpriteTexcoords = -1, vboCroppedSpriteTexcoords = -1;
	blit = NULL; i2rgb = NULL; fillRect = NULL;
}

IOGfxDisplayGL2::~IOGfxDisplayGL2() {
	close();
}

bool IOGfxDisplayGL2::open() {
	if (!IOGfxDisplay::open()) return false;

	if (!createOpenGLContext()) return false;
	if (!createSpriteVertices()) return false;
	if (!createSpriteTexcoords()) return false;
	if (!createCroppedSpriteTexcoords()) return false;
	if (!createPrograms()) return false;
	if (!createPalette()) return false;

	androidWorkAround();

	return true;
}

void IOGfxDisplayGL2::close() {
	if (gl != NULL) {
		gl->DeleteProgram(blit->program);
		gl->DeleteProgram(fillRect->program);
		gl->DeleteProgram(i2rgb->program);
		delete blit;
		delete fillRect;
		delete i2rgb;
		gl->DeleteBuffers(1, &vboSpriteVertices);
		gl->DeleteBuffers(1, &vboSpriteTexcoords);
		gl->DeleteBuffers(1, &vboCroppedSpriteTexcoords);
		delete gl;
		gl = NULL;
	}

	if (glcontext) SDL_GL_DeleteContext(glcontext);
	glcontext = NULL;

	IOGfxDisplay::close();
}

bool IOGfxDisplayGL2::createOpenGLContext() {
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	glcontext = SDL_GL_CreateContext(window);
	if (glcontext == NULL) {
		log_error("Could not create OpenGL context: %s", SDL_GetError());
		return false;
	}
	if (SDL_GL_MakeCurrent(window, glcontext) < 0) {
		SDL_GL_DeleteContext(glcontext);
		log_error("Could not make OpenGL context current: %s", SDL_GetError());
		return false;
	}

	// Let FramerateManager handle frame delay
	// TODO: drop the extra SwapWindow/RenderPresent during speed mode
	SDL_GL_SetSwapInterval(0);

	gl = new IOGfxGLFuncs();

	gl->Enable(GL_BLEND);
	//gl->Enable(GL_DEPTH_TEST);
	gl->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

bool IOGfxDisplayGL2::createSpriteVertices() {
	GLfloat spriteVertices[] = {
	    0, 0, 0, 1,
	    1, 0, 0, 1,
	    0, 1, 0, 1,
	    1, 1, 0, 1,
	};

	gl->GenBuffers(1, &vboSpriteVertices);
	gl->BindBuffer(GL_ARRAY_BUFFER, vboSpriteVertices);
	gl->BufferData(GL_ARRAY_BUFFER, sizeof(spriteVertices), spriteVertices, GL_STATIC_DRAW);
	return true;
}

bool IOGfxDisplayGL2::createSpriteTexcoords() {
	GLfloat spriteTexcoords[] = {
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	};
	gl->GenBuffers(1, &vboSpriteTexcoords);
	gl->BindBuffer(GL_ARRAY_BUFFER, vboSpriteTexcoords);
	gl->BufferData(GL_ARRAY_BUFFER, sizeof(spriteTexcoords), spriteTexcoords, GL_STATIC_DRAW);
	return true;
}

bool IOGfxDisplayGL2::createCroppedSpriteTexcoords() {
	gl->GenBuffers(1, &vboCroppedSpriteTexcoords);
	return true;
}

/**
 * Compile vertex or fragment shader.
 * 'name' is provided for debugging purposes only
 */
GLuint IOGfxDisplayGL2::createShader(const char* name, const char* source, GLenum type) {
	GLuint res = gl->CreateShader(type);

	// GLSL version
	const char* version;
	int profile;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
	if (profile == SDL_GL_CONTEXT_PROFILE_ES)
		version = "#version 100\n";  // OpenGL ES 2.0
	else
		version = "#version 120\n";  // OpenGL 2.1

	// GLES2 precision specifiers
	const char* precision;
	precision =
		"#ifdef GL_ES                        \n"
		"#  ifdef GL_FRAGMENT_PRECISION_HIGH \n"
		"     precision highp float;         \n"
		"#  else                             \n"
		"     precision mediump float;       \n"
		"#  endif                            \n"
		"#else                               \n"
		// Ignore unsupported precision specifiers
		"#  define lowp                      \n"
		"#  define mediump                   \n"
		"#  define highp                     \n"
		"#endif                              \n";

	const GLchar* sources[] = {
		version,
		precision,
		source
	};
	gl->ShaderSource(res, 3, sources, NULL);

	gl->CompileShader(res);
	GLint compile_ok = GL_FALSE;
	gl->GetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		log_error("when compiling %s %s shader:", name, (type == GL_VERTEX_SHADER) ? "vertex" : "fragment");
		infoLog(res);
		gl->DeleteShader(res);
		return 0;
	}

	return res;
}

void IOGfxDisplayGL2::infoLog(GLuint object) {
	GLint log_length = 0;
	if (gl->IsShader(object)) {
		gl->GetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else if (gl->IsProgram(object)) {
		gl->GetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else {
		log_error("logShaderInfoLog: Not a shader or a program");
		return;
	}

	char* log = (char*)malloc(log_length);

	if (gl->IsShader(object))
		gl->GetShaderInfoLog(object, log_length, NULL, log);
	else if (gl->IsProgram(object))
		gl->GetProgramInfoLog(object, log_length, NULL, log);

	log_error("%s\n", log);
	free(log);
}

bool IOGfxDisplayGL2::createPrograms() {
	blit = new IOGfxGLProg();
	blit->vshader =
		"attribute vec4 v_coord;          \n"
		"attribute vec2 v_texcoord;       \n"
		"varying vec2 f_texcoord;         \n"
		"uniform mat4 mvp;                \n"
		"                                 \n"
		"void main(void) {                \n"
		"  gl_Position = mvp * v_coord;   \n"
		"  f_texcoord = v_texcoord;       \n"
		"}                                \n";
	blit->fshader =
		"varying vec2 f_texcoord;                         \n"
		"uniform sampler2D texture;                       \n"
		"uniform vec3 colorkey;                           \n"
		"                                                 \n"
		"void main(void) {                                \n"
		"  vec4 f = texture2D(texture, f_texcoord);       \n"
		"  // don't need float comparison abs(a-b)<.001?  \n"
		"  if (f.r == colorkey.r                          \n"
		"   && f.g == colorkey.g                          \n"
		"   && f.b == colorkey.b)                         \n"
		"    f.a = 0.0;                                   \n"
		"  gl_FragColor = f;                              \n"
		"}                                                \n";
	blit->program = createProgram("blit", blit->vshader, blit->fshader);
	if ((blit->attributes["v_coord"]    = getAttribLocation(blit->program, "v_coord"))    == -1) return false;
	if ((blit->attributes["v_texcoord"] = getAttribLocation(blit->program, "v_texcoord")) == -1) return false;
	if ((blit->uniforms["mvp"]      = getUniformLocation(blit->program, "mvp"))       == -1) return false;
	if ((blit->uniforms["texture"]  = getUniformLocation(blit->program, "texture"))   == -1) return false;
	if ((blit->uniforms["colorkey"] = getUniformLocation(blit->program, "colorkey"))  == -1) return false;


	fillRect = new IOGfxGLProg();
	fillRect->vshader =
		"attribute vec4 v_coord;          \n"
		"uniform mat4 mvp;                \n"
		"                                 \n"
		"void main(void) {                \n"
		"  gl_Position = mvp * v_coord;   \n"
		"}                                \n";
	fillRect->fshader =
		"uniform vec4 color;                              \n"
		"                                                 \n"
		"void main(void) {                                \n"
		"  gl_FragColor = color;                          \n"
		"}                                                \n";
	fillRect->program = createProgram("fillRect", fillRect->vshader, fillRect->fshader);
	if ((fillRect->attributes["v_coord"] = getAttribLocation(fillRect->program,  "v_coord")) == -1) return false;
	if ((fillRect->uniforms["mvp"]   = getUniformLocation(fillRect->program, "mvp"))       == -1) return false;
	if ((fillRect->uniforms["color"] = getUniformLocation(fillRect->program, "color"))     == -1) return false;


	/* Palette emulation */
	i2rgb = new IOGfxGLProg();
	i2rgb->vshader =
		"attribute vec4 v_coord;          \n"
		"attribute vec2 v_texcoord;       \n"
		"varying vec2 f_texcoord;         \n"
		"uniform mat4 mvp;                \n"
		"                                 \n"
		"void main(void) {                \n"
		"  gl_Position = mvp * v_coord;   \n"
		"  f_texcoord = v_texcoord;       \n"
		"}                                \n";
	i2rgb->fshader =
		"varying vec2 f_texcoord;                                   \n"
		"uniform sampler2D texture;                                 \n"
		"uniform sampler2D palette;                                 \n"
		"                                                           \n"
		"void main(void) {                                          \n"
		"  vec4 index = texture2D(texture, f_texcoord);             \n"
		"  gl_FragColor = texture2D(palette, vec2(index.r, 0.5));   \n"
		"}                                                          \n";
	i2rgb->program = createProgram("i2rgb", i2rgb->vshader, i2rgb->fshader);
	if ((i2rgb->attributes["v_coord"]    = getAttribLocation(i2rgb->program, "v_coord"))    == -1) return false;
	if ((i2rgb->attributes["v_texcoord"] = getAttribLocation(i2rgb->program, "v_texcoord")) == -1) return false;
	if ((i2rgb->uniforms["mvp"]     = getUniformLocation(i2rgb->program, "mvp"))       == -1) return false;
	if ((i2rgb->uniforms["texture"] = getUniformLocation(i2rgb->program, "texture"))   == -1) return false;
	if ((i2rgb->uniforms["palette"] = getUniformLocation(i2rgb->program, "palette"))   == -1) return false;


	if (blit->program == 0 || fillRect->program == 0 || i2rgb->program == 0)
		return false;
	return true;
}

GLuint IOGfxDisplayGL2::createProgram(const char* name, const char* vertexShaderSource, const char* fragmentShaderSource) {
	GLuint vs, fs;
	if ((vs = createShader(name, vertexShaderSource,   GL_VERTEX_SHADER))   == 0) return false;
	if ((fs = createShader(name, fragmentShaderSource, GL_FRAGMENT_SHADER)) == 0) return false;

	GLuint program = gl->CreateProgram();
	if (program == 0) {
		log_error("Could not allocate program");
		return 0;
	}
	gl->AttachShader(program, vs);
	gl->AttachShader(program, fs);

	GLint link_ok = GL_FALSE;
	gl->LinkProgram(program);
	gl->GetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		infoLog(program);
		return 0;
	}

	return program;
}

GLint IOGfxDisplayGL2::getAttribLocation(GLuint program, const char* name) {
	GLint attribute = gl->GetAttribLocation(program, name);
	if (attribute == -1)
		log_error("Could not locate attribute %s", name);
	return attribute;
}
GLint IOGfxDisplayGL2::getUniformLocation(GLuint program, const char* name) {
	GLint uniform = gl->GetUniformLocation(program, name);
	if (uniform == -1)
		log_error("Could not locate uniform %s", name);
	return uniform;
}

bool IOGfxDisplayGL2::createPalette() {
	gl->GenTextures(1, &palette);
	gl->BindTexture(GL_TEXTURE_2D, palette);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	updatePalette();
	return true;  // I hereby thank OpenGL error reporting..
}

void IOGfxDisplayGL2::updatePalette() {
	SDL_Color pal_phys[256];
	gfx_palette_get_phys(pal_phys);
	gl->BindTexture(GL_TEXTURE_2D, palette);
	gl->TexImage2D(GL_TEXTURE_2D, // target
		0,       // level, 0 = base, no minimap,
		GL_RGBA, // internalformat
		256,     // width
		1,       // height
		0,       // border, always 0 in OpenGL ES
		GL_RGBA, // format
		GL_UNSIGNED_BYTE, // type
		pal_phys);
}


void IOGfxDisplayGL2::logOpenGLInfo() {
	int major, minor, profile;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
	const char* profile_str = "";
	if (profile & SDL_GL_CONTEXT_PROFILE_CORE)
		profile_str = "CORE";
	if (profile & SDL_GL_CONTEXT_PROFILE_COMPATIBILITY)
		profile_str = "COMPATIBILITY";
	if (profile & SDL_GL_CONTEXT_PROFILE_ES)
		profile_str = "ES";

	log_info("OpenGL %d.%d %s", major, minor, profile_str);
}

void IOGfxDisplayGL2::logDisplayInfo() {
	IOGfxDisplay::logDisplayInfo();
	logOpenGLInfo();
}



void IOGfxDisplayGL2::androidWorkAround() {
#ifdef __ANDROID__
	// Android/GalaxyS/CM11 is buggy (no/delayed texture blits) on startup, a full standard blit seems to help
	gl->UseProgram(blit->program);
	gl->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	SDL_GL_SwapWindow(window);
#endif
}

void IOGfxDisplayGL2::clear() {
	gl->BindFramebuffer(GL_FRAMEBUFFER, 0);
	gl->ClearColor(0,0,0,1);
	gl->Clear(GL_COLOR_BUFFER_BIT);
}

SDL_Surface* IOGfxDisplayGL2::screenshot(SDL_Rect* rect) {
	gl->BindFramebuffer(GL_FRAMEBUFFER, 0);

	// assume 4-bytes alignment
	SDL_Surface* image = SDL_CreateRGBSurface(0,
		rect->w, rect->h, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
	);
	unsigned char* pixels = (unsigned char*)image->pixels;
	gl->ReadPixels(rect->x, h-rect->h-rect->y, rect->w, rect->h,
		GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Flip vertically
    unsigned int length = rect->w*4;
    unsigned char* src = pixels + (rect->h-1)*length;
    unsigned char* tmp = new unsigned char[length];
    unsigned char* dst = pixels;
    unsigned int rows = rect->h / 2;
    while (rows--) {
        memcpy(tmp, dst, length);
        memcpy(dst, src, length);
        memcpy(src, tmp, length);
        dst += length;
        src -= length;
    }
    delete[] tmp;

	return image;
}

void IOGfxDisplayGL2::flipStretch(IOGfxSurface* backbuffer) {
	if (backbuffer == NULL)
		SDL_SetError("IOGfxDisplayGL2::flip: passed a NULL surface");
	IOGfxSurfaceGL2* surf = dynamic_cast<IOGfxSurfaceGL2*>(backbuffer);
	GLuint texture = surf->texture;

	IOGfxGLProg* prog;
	if (truecolor)
		prog = blit;
	else
		prog = i2rgb;

	gl->UseProgram(prog->program);

	gl->ActiveTexture(GL_TEXTURE0);
	gl->Uniform1i(prog->uniforms["texture"], /*GL_TEXTURE*/0);
	gl->BindTexture(GL_TEXTURE_2D, texture);
	if (!truecolor) {
		gl->ActiveTexture(GL_TEXTURE1);
		gl->Uniform1i(prog->uniforms["palette"], /*GL_TEXTURE*/1);
		gl->BindTexture(GL_TEXTURE_2D, palette);
		updatePalette();
	}

	SDL_Rect dstrect;
	centerScaledSurface(surf, &dstrect);

	// Y-inversed projection for top-left origin and top-bottom textures
	// Beware that rotation is reversed too
	glm::mat4 projection = glm::ortho(0.0f, 1.0f*w, 1.0f*h, 0.0f);
	glm::mat4 m_transform;
	m_transform = glm::translate(glm::mat4(1.0f), glm::vec3(dstrect.x,dstrect.y, 0.0))
		* glm::scale(glm::mat4(1.0f), glm::vec3(dstrect.w, dstrect.h, 0.0));
	glm::mat4 mvp = projection * m_transform; // * view * model * anim;
	gl->UniformMatrix4fv(prog->uniforms["mvp"], 1, GL_FALSE, glm::value_ptr(mvp));

	if (truecolor)
		gl->Uniform3f(prog->uniforms["colorkey"], -1,-1,-1);

	gl->EnableVertexAttribArray(prog->attributes["v_coord"]);
	// Describe our vertices array to OpenGL (it can't guess its format automatically)
	gl->BindBuffer(GL_ARRAY_BUFFER, vboSpriteVertices);
	gl->VertexAttribPointer(
		prog->attributes["v_coord"], // attribute
		4,                 // number of elements per vertex, here (x,y,z,t)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);

	gl->EnableVertexAttribArray(blit->attributes["v_texcoord"]);
	gl->BindBuffer(GL_ARRAY_BUFFER, vboSpriteTexcoords);
	gl->VertexAttribPointer(
		blit->attributes["v_texcoord"], // attribute
		2,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);

	/* Push each element in buffer_vertices to the vertex shader */
	gl->BindFramebuffer(GL_FRAMEBUFFER, 0);
	gl->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	gl->DisableVertexAttribArray(prog->attributes["v_coord"]);
	gl->DisableVertexAttribArray(blit->attributes["v_texcoord"]);

	SDL_GL_SwapWindow(window);
}

/* Raw blit so we can extract texture buffer */
void IOGfxDisplayGL2::flipDebug(IOGfxSurface* backbuffer) {
	if (backbuffer == NULL)
		SDL_SetError("IOGfxDisplayGL2::blit: passed a NULL surface");
	IOGfxSurfaceGL2* surf = dynamic_cast<IOGfxSurfaceGL2*>(backbuffer);
	GLuint texture = surf->texture;

	IOGfxGLProg* prog;
	if (truecolor)
		prog = blit;
	else
		prog = i2rgb;

	gl->UseProgram(prog->program);

	gl->ActiveTexture(GL_TEXTURE0);
	gl->Uniform1i(prog->uniforms["texture"], /*GL_TEXTURE*/0);
	gl->BindTexture(GL_TEXTURE_2D, texture);
	if (!truecolor) {
		gl->ActiveTexture(GL_TEXTURE1);
		gl->Uniform1i(prog->uniforms["palette"], /*GL_TEXTURE*/1);
		gl->BindTexture(GL_TEXTURE_2D, palette);
		updatePalette();
	}

	// Y-inversed projection for top-left origin and top-bottom textures
	// Beware that rotation is reversed too
	glm::mat4 projection = glm::ortho(0.0f, 1.0f*w, 1.0f*h, 0.0f);
	glm::mat4 m_transform;
	m_transform = glm::scale(glm::mat4(1.0f), glm::vec3(backbuffer->w, backbuffer->h, 0.0));
	glm::mat4 mvp = projection * m_transform; // * view * model * anim;
	gl->UniformMatrix4fv(prog->uniforms["mvp"], 1, GL_FALSE, glm::value_ptr(mvp));

	if (truecolor)
		gl->Uniform3f(prog->uniforms["colorkey"], -1,-1,-1);

	gl->EnableVertexAttribArray(prog->attributes["v_coord"]);
	// Describe our vertices array to OpenGL (it can't guess its format automatically)
	gl->BindBuffer(GL_ARRAY_BUFFER, vboSpriteVertices);
	gl->VertexAttribPointer(
		prog->attributes["v_coord"], // attribute
		4,                 // number of elements per vertex, here (x,y,z,t)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);

	gl->EnableVertexAttribArray(blit->attributes["v_texcoord"]);
	gl->BindBuffer(GL_ARRAY_BUFFER, vboSpriteTexcoords);
	gl->VertexAttribPointer(
		blit->attributes["v_texcoord"], // attribute
		2,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);

	/* Push each element in buffer_vertices to the vertex shader */
	gl->BindFramebuffer(GL_FRAMEBUFFER, 0);
	gl->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	gl->DisableVertexAttribArray(blit->attributes["v_coord"]);
	gl->DisableVertexAttribArray(blit->attributes["v_texcoord"]);

	SDL_GL_SwapWindow(window);
}

void IOGfxDisplayGL2::onSizeChange(int w, int h) {
	this->w = w;
	this->h = h;
	gl->Viewport(0, 0, w, h);
}

IOGfxSurface* IOGfxDisplayGL2::upload(SDL_Surface* surf) {
	GLuint texture = -1;
	gl->GenTextures(1, &texture);
	gl->BindTexture(GL_TEXTURE_2D, texture);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	log_trace("surf->w=%d", surf->w);
	log_trace("surf->h=%d", surf->h);
	log_trace("surf->format->bits=%d", surf->format->BitsPerPixel);

	// Save transparency color
	SDL_Color colorkey = {0,0,0, 0};
	Uint32 key;
	if (SDL_GetColorKey(surf, &key) == -1) {
		colorkey.a = SDL_ALPHA_OPAQUE; // no colorkey
	} else {
		if (truecolor)
			SDL_GetRGBA(key, surf->format, &colorkey.r, &colorkey.g, &colorkey.b, &colorkey.a);
		else
			colorkey.r = colorkey.g = colorkey.b = key;
		colorkey.a = SDL_ALPHA_TRANSPARENT;
	}

	if (truecolor) {
		// Dink images get alpha disabled, so use 24-bit for simplicity
		if (surf->format->BitsPerPixel != 24) {
			// don't black out transparent pixels, keep original color
			SDL_SetColorKey(surf, SDL_FALSE, 0);
			SDL_PixelFormat fmt;
			fmt.BitsPerPixel = 24;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			fmt.Rmask = 0xFF000000;
			fmt.Gmask = 0x00FF0000;
			fmt.Bmask = 0x0000FF00;
			fmt.Amask = 0x00000000;
#else
			fmt.Rmask = 0x000000FF;
			fmt.Gmask = 0x0000FF00;
			fmt.Bmask = 0x00FF0000;
			fmt.Amask = 0x00000000;
#endif
			SDL_Surface* surf24 = SDL_CreateRGBSurface(0, surf->w, surf->h, fmt.BitsPerPixel,
                    fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
			SDL_BlitSurface(surf, NULL, surf24, NULL);

			SDL_FreeSurface(surf);
			surf = surf24;
		}
		gl->TexImage2D(GL_TEXTURE_2D, // target
				0,       // level, 0 = base, no minimap,
				GL_RGB,  // internalformat
				surf->w, // width
				surf->h, // height
				0,       // border, always 0 in OpenGL ES
				GL_RGB,  // format
				GL_UNSIGNED_BYTE, // type
				surf->pixels);
	} else {
		if (surf->format->BitsPerPixel != 8) {
			log_error("IOGfxDisplayGL2::upload: format is not indexed");
			SDL_FreeSurface(surf);
			return NULL;
		}
		gl->TexImage2D(GL_TEXTURE_2D, // target
				0,       // level, 0 = base, no minimap,
				GL_LUMINANCE,  // internalformat
				surf->w, // width
				surf->h, // height
				0,       // border, always 0 in OpenGL ES
				GL_LUMINANCE,  // format
				GL_UNSIGNED_BYTE, // type
				surf->pixels);
	}

	int w = surf->w;
	int h = surf->h;

	SDL_FreeSurface(surf);
	return new IOGfxSurfaceGL2(this, texture, w, h, colorkey);
}

IOGfxSurface* IOGfxDisplayGL2::allocBuffer(int surfW, int surfH) {
	GLuint texture = -1;
	gl->GenTextures(1, &texture);
	gl->BindTexture(GL_TEXTURE_2D, texture);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// No color key
	SDL_Color colorkey = {0,0,0, SDL_ALPHA_OPAQUE};

	// Blank texture
	// RGB even in indexed mode because some setup (Android) don't allow GL_LUMINANCE as framebuffer
	unsigned char* pixels = new unsigned char[(surfW+(4-surfW%4))*4 * surfH];
	gl->TexImage2D(GL_TEXTURE_2D, // target
			0,       // level, 0 = base, no minimap,
			GL_RGB,  // internalformat
			surfW, // width
			surfH, // height
			0,       // border, always 0 in OpenGL ES
			GL_RGB,  // format
			GL_UNSIGNED_BYTE, // type
			pixels);
	delete[] pixels;

	return new IOGfxSurfaceGL2(this, texture, surfW, surfH, colorkey);
}

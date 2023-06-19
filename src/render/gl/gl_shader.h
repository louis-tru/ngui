// @private head
/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark_render_gl_glsl_shader__
#define __quark_render_gl_glsl_shader__

#include "../../util/util.h"

#if Qk_iOS
# include <OpenGLES/ES3/gl.h>
# include <OpenGLES/ES3/glext.h>
#elif Qk_OSX
#define GL_SILENCE_DEPRECATION 1
# include <OpenGL/gl3.h>
# include <OpenGL/gl3ext.h>
#elif Qk_ANDROID || Qk_LINUX
# define GL_GLEXT_PROTOTYPES
# include <GLES3/gl3.h>
# include <GLES3/gl3ext.h>
#else
# error "The operating system does not support"
#endif

#if Qk_OSX
#define Qk_GL_Version "330 core"
#else // ios es
#define Qk_GL_Version "300 es"
#endif

namespace qk {

	struct GLShaderAttr {
		const char *name;
		GLint size;
		GLenum type;
		GLsizei stride;
	};

	struct GLSLShader {
		GLuint shader, vao, vbo;
		void         use(GLsizeiptr size, const GLvoid* data);
		virtual void build() = 0;
	};

	void gl_compile_link_shader(
		GLSLShader *s,
		cChar *name, cString& vertexShader, cString& fragmentShader,
		const Array<GLShaderAttr> &attributes, cChar *uniforms);

}

#endif
/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#ifndef __flare__ios_gl__
#define __flare__ios_gl__

#import "../util/macros.h"

#if FX_IOS

#import "flare/gl/gl.h"
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>

namespace flare {

	class FX_EXPORT GLDrawProxy {
		public:
		GLDrawProxy(GLDraw* host, EAGLContext* ctx);
		~GLDrawProxy();
		void commit_render();
		GLint get_gl_texture_pixel_format(PixelData::Format pixel_format);
		void gl_main_render_buffer_storage();
		void set_surface_view(UIView* view, CAEAGLLayer* layer);
		bool refresh_surface_size(::Rect rect);
		inline GLDraw* host() { return _host; }
		static GLDrawProxy* create(Application* host, cJSON& options);
		private:
		UIView* _surface_view;
		CAEAGLLayer* _layer;
		EAGLContext* _context;
		GLDraw*      _host;
	};

}

#endif
#endif
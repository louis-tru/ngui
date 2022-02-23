// @private head
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

#ifndef __ftr__render_opengl__
#define __ftr__render_opengl__

#include "./render.h"

namespace flare {

	class OpenGLRender: public Render {
		public:
			virtual ~OpenGLRender();
			virtual SkSurface* surface() override;
			virtual bool is_gpu() override { return true; }
			virtual void reload() override;
			virtual void submit() override;
		protected:
			virtual void renderbufferStorage(uint32_t target);
			virtual void swapBuffers() = 0;
			OpenGLRender(Application* host, const Options& opts);
			sk_sp<SkSurface> _surface;
			uint32_t  _render_buffer, _frame_buffer;
			uint32_t  _msaa_render_buffer, _msaa_frame_buffer;
			bool _is_support_multisampled;
	};

	class RasterOpenGLRender: public OpenGLRender {
		public:
			virtual SkSurface* surface() override;
			virtual bool is_gpu() override { return false; }
			virtual void reload() override;
			virtual void submit() override;
		protected:
			RasterOpenGLRender(Application* host, const Options& opts);
			sk_sp<SkSurface> _rasterSurface;
	};

}
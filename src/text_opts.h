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

#ifndef __noug__text_opts__
#define __noug__text_opts__

#include "./value.h"

namespace noug {

	class FontStyle;
	class FontPool;
	class TextConfig;

	class N_EXPORT TextOptions {
	public:
		N_DEFINE_PROP(TextWeight, text_weight);
		N_DEFINE_PROP(TextSlant, text_slant);
		N_DEFINE_PROP(TextDecoration, text_decoration);
		N_DEFINE_PROP(TextOverflow, text_overflow);
		N_DEFINE_PROP(TextWhiteSpace, text_white_space);
		N_DEFINE_PROP(TextSize, text_size); // TextValueWrap
		N_DEFINE_PROP(TextColor, text_background_color);
		N_DEFINE_PROP(TextColor, text_color);
		N_DEFINE_PROP(TextShadow, text_shadow);
		N_DEFINE_PROP(TextLineHeight, text_line_height);
		N_DEFINE_PROP(TextFamily, text_family);
	protected:
		virtual void onTextChange(uint32_t mark, uint32_t flags);
		friend class TextConfig;
	};

	class N_EXPORT TextConfig {
	public:
		TextConfig(TextOptions* textSet, TextConfig* base);
		// define accseeor
		TextWeight text_weight();
		TextSlant  text_slant();
		TextDecoration text_decoration();
		TextOverflow   text_overflow();
		TextWhiteSpace text_white_space();
		float text_size();
		Color text_background_color();
		Color text_color();
		Shadow text_shadow();
		float text_line_height();
		FFID  text_family();
		FontStyle font_style();
		inline TextOptions* textOptions() const { return _textOptions; };
		inline TextConfig* base() const { return _base; };
	protected:
		TextOptions*  _textOptions;
		TextConfig*   _base; // prototype chain accessor
		uint32_t   _flags;
		TextWeight _text_weight;
		TextSlant _text_slant;
		TextDecoration _text_decoration;
		TextOverflow _text_overflow;
		TextWhiteSpace _text_white_space;
	};

	class N_EXPORT DefaultTextOptions: public TextOptions, public TextConfig {
	public:
		DefaultTextOptions(FontPool *pool);
		~DefaultTextOptions();
	protected:
		virtual void onTextChange(uint32_t mark, uint32_t flags);
	};

}
#endif
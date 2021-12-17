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

#include "./image_source.h"
#include "./util/fs.h"

namespace flare {

	/**
	* @func pixel_bit_size
	*/
	uint32_t PixelData::pixel_bit_size(ColorType type) {
		switch ( type ) {
			default:
			case COLOR_TYPE_INVALID: return 0;
			case COLOR_TYPE_ALPHA_8: return 8;      //!< pixel with alpha in 8-bit byte
			case COLOR_TYPE_RGB_565: return 16;      //!< pixel with 5 bits red, 6 bits green, 5 bits blue, in 16-bit word
			case COLOR_TYPE_ARGB_4444: return 16;    //!< pixel with 4 bits for alpha, red, green, blue; in 16-bit word
			case COLOR_TYPE_RGBA_8888: return 32;    //!< pixel with 8 bits for red, green, blue, alpha; in 32-bit word
			case COLOR_TYPE_RGB_888X: return 32;     //!< pixel with 8 bits each for red, green, blue; in 32-bit word
			case COLOR_TYPE_BGRA_8888: return 32;    //!< pixel with 8 bits for blue, green, red, alpha; in 32-bit word
			case COLOR_TYPE_RGBA_1010102: return 10; //!< 10 bits for red, green, blue; 2 bits for alpha; in 32-bit word
			case COLOR_TYPE_BGRA_1010102: return 10; //!< 10 bits for blue, green, red; 2 bits for alpha; in 32-bit word
			case COLOR_TYPE_RGB_101010X: return 10;  //!< pixel with 10 bits each for red, green, blue; in 32-bit word
			case COLOR_TYPE_BGR_101010X: return 10;  //!< pixel with 10 bits each for blue, green, red; in 32-bit word
			case COLOR_TYPE_GRAY_8: return 8;       //!< pixel with grayscale level in 8-bit byte
		}
	}

	PixelData PixelData::decode(Buffer raw) {
		// TODO ...
		return PixelData();
	}

	PixelData::PixelData()
		: _data()
		, _width(0)
		, _height(0)
		, _body()
		, _type(COLOR_TYPE_INVALID) {
	}

	PixelData::PixelData(cPixelData& body)
		: _data()
		, _width(body._width)
		, _height(body._height)
		, _body(body._body)
		, _type(body._type) {
	}

	PixelData::PixelData(PixelData&& body)
		: _data(body._data)
		, _width(body._width)
		, _height(body._height)
		, _body(std::move(body._body))
		, _type(body._type) {
	}

	PixelData::PixelData(ColorType type)
		: _data()
		, _width(0)
		, _height(0)
		, _body()
		, _type(type) {
	}

	PixelData::PixelData(Buffer body, int width, int height, ColorType type)
		: _data(body)
		, _width(width)
		, _height(height)
		, _body()
		, _type(type) {
		_body.push(WeakBuffer(*_data, _data.length()));
	}

	PixelData::PixelData(WeakBuffer body, int width, int height, ColorType type)
		: _data()
		, _width(width)
		, _height(height)
		, _body()
		, _type(type) {
		_body.push(body);
	}

	PixelData::PixelData(const Array<WeakBuffer>& body, int width, int height, ColorType type)
		: _data()
		, _width(width)
		, _height(height)
		, _body(body)
		, _type(type) {
	}
	
	ImageSource::ImageSource(cString& uri)
		: F_Init_Event(State)
		, _id(fs_reader()->format(uri))
		, _state(STATE_NONE)
		, _inl(nullptr)
	{
	}

	ImageSource::ImageSource(PixelData pixel)
		: F_Init_Event(State)
		, _state(STATE_NONE)
		, _pixel(pixel)
		, _inl(nullptr)
	{
		_id = String("image_").append(getId());
	}

	ImageSource::~ImageSource() {
		// TODO ...
	}

	/**
		* 
		* mark as gpu texture
		*
		* @func mark_as_texture()
		*/
	bool ImageSource::mark_as_texture() {
		// TODO ...
		return false;
	}

	/**
		* @func load() async load image source
		*/
	bool ImageSource::load() {
		if (_state & STATE_LOAD_COMPLETE) {
			return true;
		}
		// TODO ...
		return false;
	}

	/**
		* @func ready() async ready
		*/
	bool ImageSource::ready() {
		if (is_ready()) {
			return true;
		}
		// TODO ...
		return false;
	}

	/**
		* @func unload() delete load and ready
		*/
	void ImageSource::unload() {
		// TODO ...
	}

	ColorType ImageSource::type() const {
		// TODO ...
	}

	int ImageSource::width() const {
		// TODO ...
	}

	int ImageSource::height() const {
		// TODO ...
	}


	void ImagePool::clear(bool full) {
		// TODO ..
	}

}

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

#include "./hash.h"

namespace qk {

	static cChar* I64BIT_TABLE =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

	void Hash5381::update(cVoid* data, uint32_t len) {
		while (len--)
			_hash += (_hash << 5) + reinterpret_cast<const uint8_t*>(data)[len];
	}

	void Hash5381::updateu16v(const uint16_t *data, uint32_t len) {
		while (len--)
			_hash += (_hash << 5) + data[len];
	}

	void Hash5381::updateu32v(const uint32_t *data, uint32_t len) {
		while (len--)
			_hash += (_hash << 5) + data[len];
	}

	void Hash5381::updateu64v(const uint64_t *data, uint32_t len) {
		while (len--)
			_hash += (_hash << 5) + data[len]; // update u64
	}

	void Hash5381::updateu64(const uint64_t data) {
		_hash += (_hash << 5) + data; // update u64
	}

	void Hash5381::updateu32(const uint32_t data) {
		_hash += (_hash << 5) + data; // update u32
	}

	void Hash5381::updatef(float data) {
		_hash += (_hash << 5) + *reinterpret_cast<const uint64_t*>(&data); // update
	}

	void Hash5381::updatefv2(const float data[2]) {
		_hash += (_hash << 5) + *reinterpret_cast<const uint64_t*>(data); // update
	}

	void Hash5381::updatefv4(const float data[4]) {
		_hash += (_hash << 5) + *reinterpret_cast<const uint64_t*>(data); // update u64
		_hash += (_hash << 5) + *reinterpret_cast<const uint64_t*>(data+2); // update u64
	}

	void Hash5381::updatestr(cString& str) {
		update(str.c_str(), str.length());
	}

	String Hash5381::digest() {
		String rev;
		do {
			rev.append(I64BIT_TABLE[_hash & 0x3F]);
		} while (_hash >>= 6);
		_hash = 5381;
		return rev;
	}

	uint64_t hashCode(cVoid* data, uint32_t len) {
		Hash5381 hash;
		hash.update(data, len);
		return hash.hashCode();
	}

	String hash(cVoid* data, uint32_t len) {
		Hash5381 hash;
		hash.update((cChar*)data, len);
		return hash.digest();
	}

	String hash(cString& str) {
		return hash(str.c_str(), str.length());
	}

}

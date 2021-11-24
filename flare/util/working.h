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


#ifndef __flare__util___working__
#define __flare__util___working__

#include "./loop.h"
#include "./dict.h"

namespace flare {

	/**
	* @class ParallelWorking
	*/
	class F_EXPORT ParallelWorking: public Object {
		F_HIDDEN_ALL_COPY(ParallelWorking);
	 public:
		typedef Thread::Exec Exec;
		ParallelWorking();
		ParallelWorking(RunLoop* loop);
		virtual ~ParallelWorking();
		ThreadID spawn_child(Exec exec, cString& name);
		void awaken_child(ThreadID id = ThreadID());  // default awaken all child
		void abort_child(ThreadID id = ThreadID());   // default abort all child
		uint32_t post(Cb cb); // post message to main thread
		uint32_t post(Cb cb, uint64_t delay_us);
		void cancel(uint32_t id = 0); // cancel message
	 private:
		typedef Dict<ThreadID, int> Childs;
		KeepLoop* _proxy;
		Mutex _mutex2;
		Childs _childs;
	};

	F_DEFINE_INLINE_MEMBERS(RunLoop, Inl2) {
	 public:
		inline void set_independent_mutex(RecursiveMutex* mutex) {
			_independent_mutex = mutex;
		}
		inline void independent_mutex_lock() {
			if (_independent_mutex) {
				_independent_mutex->lock();
			}
		}
		inline void independent_mutex_unlock() {
			if (_independent_mutex) {
				_independent_mutex->unlock();
			}
		}
	};

}
#endif
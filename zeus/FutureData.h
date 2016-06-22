/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <etk/types.h>
#include <zeus/Buffer.h>
#include <functional>
#include <ememory/memory.h>


namespace zeus {
	class FutureBase;
	class FutureData {
		public:
			using ObserverFinish = std::function<bool(zeus::FutureBase)>; //!< Define an Observer: function pointer
		public:
			uint64_t m_transactionId;
			bool m_isSynchronous;
			bool m_isFinished;
			ememory::SharedPtr<zeus::Buffer> m_returnData;
			ObserverFinish m_callbackFinish;
			std::chrono::steady_clock::time_point m_sendTime;
			std::chrono::steady_clock::time_point m_receiveTime;
	};
}


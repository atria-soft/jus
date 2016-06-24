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
	/**
	 * @brief Data interface of the future (the future can be copied, but the data need to stay...
	 */
	class FutureData {
		public:
			using ObserverFinish = std::function<bool(zeus::FutureBase)>; //!< Define an Observer: function pointer
		public:
			uint32_t m_transactionId; //!< waiting answer data
			uint32_t m_clientId; //!< need to anser at this client.
			bool m_isSynchronous; //!< the future is synchronous. (call when receive data)
			ememory::SharedPtr<zeus::Buffer> m_returnData; //!< all buffer concatenate or last buffer if synchronous
			ObserverFinish m_callbackFinish; //!< ofserver of the finish data
			std::chrono::steady_clock::time_point m_sendTime; //!< time when the future has been sended request
			std::chrono::steady_clock::time_point m_receiveTime; //!< time when the future has receve answer
	};
}


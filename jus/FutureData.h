/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <etk/types.h>
#include <ejson/ejson.h>

namespace jus {
	class FutureBase;
	class FutureData {
		public:
			using ObserverFinish = std::function<bool(jus::FutureBase)>; //!< Define an Observer: function pointer
		public:
			uint64_t m_transactionId;
			bool m_isSynchronous;
			bool m_isFinished;
			ejson::Object m_returnData;
			std::vector<ejson::Value> m_returnDataPart;
			ObserverFinish m_callbackFinish;
			std::chrono::steady_clock::time_point m_sendTime;
			std::chrono::steady_clock::time_point m_receiveTime;
	};
}


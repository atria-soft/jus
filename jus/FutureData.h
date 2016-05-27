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
			using ObserverFinish = std::function<void(jus::FutureBase)>; //!< Define an Observer: function pointer
		public:
			uint64_t m_transactionId;
			bool m_isFinished;
			ejson::Object m_returnData;
			ObserverFinish m_callbackFinish;
	};
}


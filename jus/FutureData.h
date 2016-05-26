/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <etk/types.h>
#include <ejson/ejson.h>

namespace jus {
	class FutureData {
		public:
			uint64_t m_transactionId;
			bool m_isFinished;
			ejson::Object m_returnData;
	};
}


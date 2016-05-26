/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/FutureData.h>

namespace jus {
	class FutureBase {
		public:
			ememory::SharedPtr<jus::FutureData> m_data;
		public:
			FutureBase(const jus::FutureBase& _base);
			FutureBase();
			FutureBase(uint64_t _transactionId);
			FutureBase(uint64_t _transactionId, bool _isFinished, ejson::Object _returnData);
			jus::FutureBase operator= (const jus::FutureBase& _base);
			void setAnswer(const ejson::Object& _returnValue);
			uint64_t getTransactionId();
			bool hasError();
			std::string getErrorType();
			std::string getErrorHelp();
			bool isValid();
			bool isFinished();
			void wait();
			bool waitFor(std::chrono::microseconds _delta);
			bool waitUntil(std::chrono::steady_clock::time_point _endTime);
	};
}


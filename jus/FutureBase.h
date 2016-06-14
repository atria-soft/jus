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
			FutureBase(uint64_t _transactionId, jus::FutureData::ObserverFinish _callback=nullptr);
			FutureBase(uint64_t _transactionId, bool _isFinished, ejson::Object _returnData, jus::FutureData::ObserverFinish _callback=nullptr);
			FutureBase(uint64_t _transactionId, bool _isFinished, jus::Buffer _returnData, jus::FutureData::ObserverFinish _callback=nullptr);
			jus::FutureBase operator= (const jus::FutureBase& _base);
			bool setAnswer(const ejson::Object& _returnValue);
			bool setAnswer(const jus::Buffer& _returnValue);
			void setSynchronous();
			uint64_t getTransactionId();
			bool hasError();
			std::string getErrorType();
			std::string getErrorHelp();
			bool isValid();
			bool isFinished();
			FutureBase& wait();
			FutureBase& waitFor(std::chrono::microseconds _delta = std::chrono::seconds(30));
			FutureBase& waitUntil(std::chrono::steady_clock::time_point _endTime);
			const jus::Buffer& getRaw();
			std::chrono::nanoseconds getTransmitionTime();
	};
	class FutureCall {
		private:
			uint64_t m_transactionId;
			uint64_t m_clientId;
			bool m_isFinished;
			jus::Buffer m_data;
			std::vector<jus::Buffer> m_dataMultiplePack;
			std::chrono::steady_clock::time_point m_receiveTime;
			std::chrono::steady_clock::time_point m_answerTime;
		public:
			FutureCall(uint64_t _clientId, uint64_t _transactionId, jus::Buffer& _callValue);
			void appendData(jus::Buffer& _callValue);
			uint64_t getTransactionId();
			uint64_t getClientId();
			bool isFinished();
			std::chrono::nanoseconds getTransmitionTime();
			jus::Buffer& getRaw();
	};
}


/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/FutureData.h>

namespace zeus {
	class FutureBase {
		public:
			ememory::SharedPtr<zeus::FutureData> m_data;
		public:
			FutureBase(const zeus::FutureBase& _base);
			FutureBase();
			FutureBase(uint64_t _transactionId, zeus::FutureData::ObserverFinish _callback=nullptr);
			FutureBase(uint64_t _transactionId, bool _isFinished, const ememory::SharedPtr<zeus::Buffer>& _returnData, zeus::FutureData::ObserverFinish _callback=nullptr);
			zeus::FutureBase operator= (const zeus::FutureBase& _base);
			bool setAnswer(const ememory::SharedPtr<zeus::Buffer>& _returnValue);
			void setSynchronous();
			uint64_t getTransactionId();
			bool hasError();
			std::string getErrorType();
			std::string getErrorHelp();
			bool isValid() const;
			bool isFinished() const;
			FutureBase& wait();
			FutureBase& waitFor(std::chrono::microseconds _delta = std::chrono::seconds(30));
			FutureBase& waitUntil(std::chrono::steady_clock::time_point _endTime);
			ememory::SharedPtr<zeus::Buffer> getRaw();
			std::chrono::nanoseconds getTransmitionTime();
	};
	class FutureCall {
		private:
			uint64_t m_transactionId;
			uint64_t m_clientId;
			bool m_isFinished;
			ememory::SharedPtr<zeus::Buffer> m_data;
			std::vector<ememory::SharedPtr<zeus::Buffer>> m_dataMultiplePack;
			std::chrono::steady_clock::time_point m_receiveTime;
			std::chrono::steady_clock::time_point m_answerTime;
		public:
			FutureCall(uint64_t _clientId, uint64_t _transactionId, const ememory::SharedPtr<zeus::Buffer>& _callValue);
			void appendData(const ememory::SharedPtr<zeus::Buffer>& _callValue);
			uint64_t getTransactionId() const;
			uint64_t getClientId() const;
			bool isFinished() const;
			std::chrono::nanoseconds getTransmitionTime() const;
			ememory::SharedPtr<zeus::Buffer> getRaw() const;
	};
}


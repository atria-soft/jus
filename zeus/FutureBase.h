/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/FutureData.h>

namespace zeus {
	/**
	 * @brief Generic zeus Future interface to get data asynchronously
	 */
	class FutureBase {
		public:
			ememory::SharedPtr<zeus::FutureData> m_data; // Reference on the data
		public:
			/**
			 * @brief Copy contructor of a FutureBase
			 * @param[in] _base the FutureBase to copy
			 */
			FutureBase(const zeus::FutureBase& _base);
			/**
			 * @brief contructor of a FutureBase
			 */
			FutureBase();
			/**
			 * @brief Contructor of the FutureBase with an ofserver
			 * @param[in] _transactionId Transaction waiting answer
			 * @param[in] _callback Observer pointer
			 */
			FutureBase(uint64_t _transactionId, zeus::FutureData::ObserverFinish _callback=nullptr);
			/**
			 * @brief Contructor of the FutureBase for direct error answer
			 * @param[in] _transactionId Transaction waiting answer
			 * @param[in] _isFinished set state finish or not
			 * @param[in] _returnData Set return value
			 * @param[in] _callback Observer pointer
			 * @return 
			 */
			FutureBase(uint64_t _transactionId, bool _isFinished, const ememory::SharedPtr<zeus::Buffer>& _returnData, zeus::FutureData::ObserverFinish _callback=nullptr);
			/**
			 * @brief Asignement operator with an other future
			 * @param[in] _base Generic base Future
			 * @return the reference on the local element
			 */
			zeus::FutureBase operator= (const zeus::FutureBase& _base);
			/**
			 * @brief specify answer of the call
			 * @param[in] _returnValue Returned buffer
			 * @return return true if an error occured
			 */
			bool setAnswer(const ememory::SharedPtr<zeus::Buffer>& _returnValue);
			/**
			 * @brief Set the future syncronous
			 * @note this mean that the system call the observer every time a packet arrive in the Future
			 */
			void setSynchronous();
			/**
			 * @brief Get the transaction Id of the Future
			 * @return Transaction Id requested or 0
			 */
			uint64_t getTransactionId();
			/**
			 * @brief check if the answer have an error
			 * @return return true if an error is registered
			 */
			bool hasError();
			/**
			 * @brief get type of the error
			 * @return the string of the error type
			 */
			std::string getErrorType();
			/**
			 * @brief get help of the error
			 * @return the string of the error help
			 */
			std::string getErrorHelp();
			/**
			 * @brief Check if the Futur is a valid data
			 * @return return true if the data is valid
			 */
			bool isValid() const;
			/**
			 * @brief Check if the futur have finish receiving data
			 * @return status of the fisnish state
			 */
			bool isFinished() const;
			/**
			 * @brief Wait the Future receive data
			 * @return reference on the current futur
			 */
			FutureBase& wait();
			/**
			 * @brief Wait the Future receive data
			 * @param[in] _delta delay to wait the data arrive
			 * @return reference on the current futur
			 */
			FutureBase& waitFor(std::chrono::microseconds _delta = std::chrono::seconds(30));
			/**
			 * @brief Wait the Future receive data
			 * @param[in] _endTime tiem to wait the data
			 * @return reference on the current futur
			 */
			FutureBase& waitUntil(std::chrono::steady_clock::time_point _endTime);
			/**
			 * @brief Get the Buffer receive
			 * @return pointer on the receive data
			 */
			ememory::SharedPtr<zeus::Buffer> getRaw();
			/**
			 * @brief Get duration of the current trasaction take
			 * @return Tile in nanosecond to wait answer
			 */
			std::chrono::nanoseconds getTransmitionTime();
	};
	/**
	 * @brief Receiving call futur
	 */
	class FutureCall {
		private:
			uint64_t m_transactionId;
			uint64_t m_clientId;
			bool m_isFinished;
			ememory::SharedPtr<zeus::Buffer> m_data;
			std::chrono::steady_clock::time_point m_receiveTime;
			std::chrono::steady_clock::time_point m_answerTime;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			FutureCall(uint64_t _clientId, uint64_t _transactionId, const ememory::SharedPtr<zeus::Buffer>& _callValue);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void appendData(const ememory::SharedPtr<zeus::Buffer>& _callValue);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			uint64_t getTransactionId() const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			uint64_t getClientId() const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool isFinished() const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			std::chrono::nanoseconds getTransmitionTime() const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			ememory::SharedPtr<zeus::Buffer> getRaw() const;
	};
}


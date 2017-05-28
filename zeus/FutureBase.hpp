/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/Promise.hpp>

namespace zeus {
	/**
	 * @brief Generic zeus Future interface to get data asynchronously
	 */
	class FutureBase {
		public:
			ememory::SharedPtr<zeus::Promise> m_promise; // Reference on the data
		public:
			/**
			 * @brief Copy contructor of a FutureBase
			 * @param[in] _base the FutureBase to copy
			 */
			FutureBase(const zeus::FutureBase& _base);
			/**
			 * @brief Copy contructor of a FutureBase
			 * @param[in] _base the FutureBase to copy
			 */
			FutureBase(ememory::SharedPtr<zeus::Promise> _promise);
			/**
			 * @brief contructor of a FutureBase
			 */
			FutureBase();
			/**
			 * @brief Contructor of the FutureBase with an ofserver
			 * @param[in] _transactionId Transaction waiting answer
			 * @param[in] _source Client/sevice Id waiting answer
			 */
			FutureBase(uint32_t _transactionId, uint32_t _source=0);
			/**
			 * @brief Contructor of the FutureBase for direct error answer
			 * @param[in] _transactionId Transaction waiting answer
			 * @param[in] _isFinished set state finish or not
			 * @param[in] _returnData Set return value
			 * @param[in] _source Source that is waiting for answer
			 */
			FutureBase(uint32_t _transactionId, ememory::SharedPtr<zeus::Message> _returnData, uint32_t _source=0);
			/**
			 * @brief set the call is an action an then it can receive remote data ==> the authorize the onProgress Callback ..
			 * @note system use only ==> user have never to call this function...
			 */
			void setAction();
			/**
			 * @brief Attach callback on all return type of value
			 * @param[in] _callback Handle on the function to call in all case
			 */
			void andAll(zeus::Promise::Observer _callback);
			/**
			 * @brief Attach callback on a specific return action (SUCESS)
			 * @param[in] _callback Handle on the function to call in case of sucess on the call
			 */
			void andThen(zeus::Promise::Observer _callback);
			/**
			 * @brief Attach callback on a specific return action (ERROR)
			 * @param[in] _callback Handle on the function to call in case of error on the call
			 */
			void andElse(zeus::Promise::Observer _callback);
			/**
			 * @brief Attach callback on activity of the action if user set some return information
			 * @param[in] _callback Handle on the function to call in progress information
			 */
			void onProgress(zeus::Promise::ObserverProgress _callback);
			/*
			/ **
			 * @brief Attach callback on a specific return action (ABORT)
			 * @param[in] _callback Handle on the function to call in case of abort on the call
			 * /
			void andAbort(zeus::Promise::Observer _callback); // an abort is  requested in the actiron ...
			*/
			/**
			 * @brief Asignement operator with an other future
			 * @param[in] _base Generic base Future
			 * @return the reference on the local element
			 */
			zeus::FutureBase operator= (const zeus::FutureBase& _base);
			/**
			 * @brief Add data on the call/answer
			 * @param[in] _returnValue Returned buffer
			 * @return return true if an error occured
			 */
			bool setMessage(ememory::SharedPtr<zeus::Message> _returnValue);
			/**
			 * @brief Get the transaction Id of the Future
			 * @return Transaction Id requested or 0
			 */
			uint32_t getTransactionId() const;
			/**
			 * @brief Get the client Id of the Future
			 * @return Client id requested or 0
			 */
			uint32_t getSource() const;
			/**
			 * @brief The remote object is removed ==> no need to wait more ...
			 */
			void remoteObjectDestroyed();
			/**
			 * @brief check if the answer have an error
			 * @return return true if an error is registered
			 */
			bool hasError() const;
			/**
			 * @brief get type of the error
			 * @return the string of the error type
			 */
			std::string getErrorType() const;
			/**
			 * @brief get help of the error
			 * @return the string of the error help
			 */
			std::string getErrorHelp() const;
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
			const FutureBase& wait() const;
			/**
			 * @brief Wait the Future receive data
			 * @param[in] _delta delay to wait the data arrive
			 * @return reference on the current futur
			 */
			const FutureBase& waitFor(echrono::Duration _delta = echrono::seconds(30)) const;
			/**
			 * @brief Wait the Future receive data
			 * @param[in] _endTime tiem to wait the data
			 * @return reference on the current futur
			 */
			const FutureBase& waitUntil(echrono::Steady _endTime) const;
			/**
			 * @brief Get the Message receive
			 * @return pointer on the receive data
			 */
			ememory::SharedPtr<zeus::Message> getRaw();
			/**
			 * @brief Get duration of the current trasaction take
			 * @return Tile in nanosecond to wait answer
			 */
			echrono::Duration getTransmitionTime() const;
	};
}


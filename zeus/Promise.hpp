/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/types.hpp>
#include <etk/Function.hpp>
#include <zeus/message/Message.hpp>
#include <zeus/message/Event.hpp>
#include <ememory/memory.hpp>
#include <echrono/Steady.hpp>
#include <echrono/Duration.hpp>


namespace zeus {
	class FutureBase;
	/**
	 * @brief Data interface of the future (the future can be copied, but the data need to stay...)
	 */
	class Promise : public ememory::EnableSharedFromThis<zeus::Promise> {
		public:
			using Observer = etk::Function<bool(zeus::FutureBase)>; //!< Define an Observer: function pointer
			using ObserverEvent = etk::Function<void(ememory::SharedPtr<zeus::message::Event>)>; //!< Define the observer on activity of the action (note that is a string, but it can contain json or other ...)
		private:
			mutable ethread::Mutex m_mutex; //!< local prevention of multiple acess
			uint32_t m_transactionId; //!< waiting answer data
			uint32_t m_source; //!< Source of the message.
			ememory::SharedPtr<zeus::Message> m_message; //!< all buffer concatenate or last buffer if synchronous
			Observer m_callbackThen; //!< observer callback When data arrive and NO error appear
			Observer m_callbackElse; //!< observer callback When data arrive and AN error appear
			ObserverEvent m_callbackEvent; //!< observer callback when event is sended from the remote object called
			//Observer m_callbackAbort; //!< observer callback When Action is abort by user
			echrono::Steady m_sendTime; //!< time when the future has been sended request
			echrono::Steady m_receiveTime; //!< time when the future has receve answer
			// TODO: Chek if it is not good to set it in DEBUG only ....
			bool m_isAction; //!< Permit to filter the user setting a callbak that is never called ==> cosmetc usage
		public:
			/**
			 * @brief Contructor of the FutureBase with an ofserver
			 * @param[in] _transactionId Transaction waiting answer
			 * @param[in] _source Client/sevice Id waiting answer
			 */
			Promise(uint32_t _transactionId, uint32_t _source=0);
			/**
			 * @brief Contructor of the FutureBase for direct error answer
			 * @param[in] _transactionId Transaction waiting answer
			 * @param[in] _isFinished set state finish or not
			 * @param[in] _returnData Set return value
			 * @param[in] _source Source that is waiting for answer
			 */
			Promise(uint32_t _transactionId, ememory::SharedPtr<zeus::Message> _returnData, uint32_t _source=0);
			/**
			 * @brief set the call is an action an then it can receive remote data ==> the authorize the onEvent Callback ..
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
			 * @param[in] _callback Handle on the function to call in event information
			 */
			void onEvent(zeus::Promise::ObserverEvent _callback);
			/*
			/ **
			 * @brief Attach callback on a specific return action (ABORT)
			 * @param[in] _callback Handle on the function to call in case of abort on the call
			 * /
			void andAbort(zeus::Promise::Observer _callback); // an abort is  requested in the actiron ...
			*/
			/**
			 * @brief The remote object is removed ==> no need to wait more ...
			 */
			void remoteObjectDestroyed();
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
			 * @brief check if the answer have an error
			 * @return return true if an error is registered
			 */
			bool hasError() const;
			/**
			 * @brief get type of the error
			 * @return the string of the error type
			 */
			etk::String getErrorType() const;
			/**
			 * @brief get help of the error
			 * @return the string of the error help
			 */
			etk::String getErrorHelp() const;
			/**
			 * @brief Check if the futur have finish receiving data
			 * @return status of the fisnish state
			 */
			bool isFinished() const;
			/**
			 * @brief Wait the Future receive data
			 * @return reference on the current futur
			 */
			void wait() const;
			/**
			 * @brief Wait the Future receive data
			 * @param[in] _delta delay to wait the data arrive
			 * @return reference on the current futur
			 */
			void waitFor(echrono::Duration _delta = echrono::seconds(30)) const;
			/**
			 * @brief Wait the Future receive data
			 * @param[in] _endTime tiem to wait the data
			 * @return reference on the current futur
			 */
			void waitUntil(echrono::Steady _endTime) const;
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


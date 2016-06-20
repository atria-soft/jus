/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <eproperty/Value.h>
#include <esignal/Signal.h>
#include <jus/Buffer.h>
#include <enet/Tcp.h>
#include <thread>
#include <memory>
#include <jus/AbstractFunction.h>
#include <jus/FutureBase.h>

namespace jus {
	class TcpString : public eproperty::Interface {
		private:
			enet::Tcp m_connection;
			std::thread* m_thread;
			bool m_threadRunning;
			uint16_t m_transmissionId;
			uint16_t getId() {
				return m_transmissionId++;
			}
			std::mutex m_pendingCallMutex;
			std::vector<std::pair<uint64_t, jus::FutureBase>> m_pendingCall;
			std::vector<uint8_t> m_buffer;
			std::vector<uint8_t> m_temporaryBuffer;
			std::chrono::steady_clock::time_point m_lastReceive;
			std::chrono::steady_clock::time_point m_lastSend;
		public:
			using Observer = std::function<void(jus::Buffer&)>; //!< Define an Observer: function pointer
			Observer m_observerElement;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(jus::Buffer&)) {
				m_observerElement = [=](jus::Buffer& _value){
					(*_class.*_func)(_value);
				};
			}
		public:
			TcpString();
			TcpString(enet::Tcp _connection);
			virtual ~TcpString();
			void setInterface(enet::Tcp _connection);
			void connect(bool _async = false);
			void disconnect(bool _inThreadStop = false);
			bool isActive() const;
			void setInterfaceName(const std::string& _name);
			int32_t writeBinary(jus::Buffer& _data);
			std::string asyncRead();
		private:
			void read();
			
			void newBuffer(jus::Buffer& _buffer);
		private:
			void threadCallback();
		public:
			const std::chrono::steady_clock::time_point& getLastTimeReceive() {
				return m_lastReceive;
			}
			const std::chrono::steady_clock::time_point& getLastTimeSend() {
				return m_lastSend;
			}
		private:
			using ActionAsync = std::function<bool(TcpString* _interface)>;
			std::mutex m_threadAsyncMutex;
			std::thread* m_threadAsync;
			bool m_threadAsyncRunning;
			std::vector<ActionAsync> m_threadAsyncList;
		private:
			void threadAsyncCallback();
		public:
			void addAsync(ActionAsync _elem) {
				std::unique_lock<std::mutex> lock(m_threadAsyncMutex);
				m_threadAsyncList.push_back(_elem);
			}
		private:
			jus::FutureBase callBinary(uint64_t _transactionId,
			                           jus::Buffer& _obj,
			                           const std::vector<ActionAsyncClient>& _async,
			                           jus::FutureData::ObserverFinish _callback=nullptr,
			                           const uint32_t& _service=0);
		public: // section call direct
			template<class... _ARGS>
			jus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				uint16_t id = getId();
				std::vector<jus::ActionAsyncClient> asyncAction;
				jus::Buffer callElem = jus::createBinaryCall(asyncAction, id, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem, asyncAction);
			}
			template<class... _ARGS>
			jus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args, jus::FutureData::ObserverFinish _callback) {
				uint16_t id = getId();
				std::vector<jus::ActionAsyncClient> asyncAction;
				jus::Buffer callElem = jus::createBinaryCall(asyncAction, id, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem, asyncAction, _callback);
			}
		public: // section call with service ID / Client ID
			
			template<class... _ARGS>
			jus::FutureBase callService(uint32_t _serviceId, const std::string& _functionName, _ARGS&&... _args) {
				uint16_t id = getId();
				std::vector<jus::ActionAsyncClient> asyncActionToDo;
				jus::Buffer callElem = jus::createBinaryCallService(asyncActionToDo, id, _serviceId, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem, asyncActionToDo);
			}
			template<class... _ARGS>
			jus::FutureBase callServiceAction(uint32_t _serviceId, const std::string& _functionName, _ARGS&&... _args, jus::FutureData::ObserverFinish _callback) {
				uint16_t id = getId();
				std::vector<jus::ActionAsyncClient> asyncActionToDo;
				jus::Buffer callElem = jus::createBinaryCallService(asyncActionToDo, id, _serviceId, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem, asyncActionToDo, _callback);
			}
			template<class... _ARGS>
			jus::FutureBase callClient(uint32_t _clientId,
			                           const std::string& _functionName,
			                           _ARGS&&... _args) {
				return callService(_clientId, _functionName, _args...);
			}
			template<class... _ARGS>
			jus::FutureBase callClientAction(uint32_t _clientId,
			                                 const std::string& _functionName,
			                                 _ARGS&&... _args,
			                                 jus::FutureData::ObserverFinish _callback) {
				return callServiceAction(_clientId, _functionName, _args..., _callback);
			}
			jus::FutureBase callForward(uint32_t _clientId,
			                            jus::Buffer& _Buffer,
			                            uint64_t _singleReferenceId,
			                            jus::FutureData::ObserverFinish _callback);
			void callForwardMultiple(uint32_t _clientId,
			                         jus::Buffer& _Buffer,
			                         uint64_t _singleReferenceId);
		public: // answers ...
			
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			template<class JUS_ARG>
			void answerValue(uint64_t _clientTransactionId, JUS_ARG _value, uint32_t _clientId=0) {
				jus::Buffer answer;
				answer.setType(jus::Buffer::typeMessage::answer);
				answer.setTransactionId(_clientTransactionId);
				answer.setClientId(_clientId);
				answer.addAnswer(_value);
				writeBinary(answer);
			}
			void answerVoid(uint64_t _clientTransactionId, uint32_t _clientId=0);
			void answerError(uint64_t _clientTransactionId, const std::string& _errorValue, const std::string& _errorComment="", uint32_t _clientId=0);
	};
}

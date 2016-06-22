/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <eproperty/Value.h>
#include <esignal/Signal.h>
#include <zeus/Buffer.h>
#include <enet/WebSocket.h>
#include <thread>
#include <memory>
#include <zeus/AbstractFunction.h>
#include <zeus/FutureBase.h>

namespace zeus {
	class TcpString : public eproperty::Interface {
		private:
			enet::WebSocket m_connection;
			uint16_t m_transmissionId;
			uint16_t getId() {
				return m_transmissionId++;
			}
			std::mutex m_pendingCallMutex;
			std::vector<std::pair<uint64_t, zeus::FutureBase>> m_pendingCall;
		public:
			using Observer = std::function<void(const ememory::SharedPtr<zeus::Buffer>&)>; //!< Define an Observer: function pointer
			Observer m_observerElement;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(const ememory::SharedPtr<zeus::Buffer>&)) {
				m_observerElement = [=](const ememory::SharedPtr<zeus::Buffer>& _value){
					(*_class.*_func)(_value);
				};
			}
		public:
			TcpString();
			TcpString(enet::Tcp _connection, bool _isServer);
			virtual ~TcpString();
			void setInterface(enet::Tcp _connection, bool _isServer);
			void connect(bool _async = false);
			void disconnect(bool _inThreadStop = false);
			bool isActive() const;
			void setInterfaceName(const std::string& _name);
			int32_t writeBinary(const ememory::SharedPtr<zeus::Buffer>& _data);
			void ping();
			bool onReceiveUri(const std::string& _uri, const std::vector<std::string>& _protocols);
			void onReceiveData(std::vector<uint8_t>& _frame, bool _isBinary);
			void newBuffer(const ememory::SharedPtr<zeus::Buffer>& _buffer);
		public:
			const std::chrono::steady_clock::time_point& getLastTimeReceive() {
				return m_connection.getLastTimeReceive();
			}
			const std::chrono::steady_clock::time_point& getLastTimeSend() {
				return m_connection.getLastTimeSend();
			}
		private:
			using ActionAsync = std::function<bool(TcpString* _interface)>;
			std::mutex m_threadAsyncMutex;
			std::thread* m_threadAsync;
			bool m_threadAsyncRunning;
			std::vector<ActionAsync> m_threadAsyncList;
			std::vector<ActionAsync> m_threadAsyncList2;
		private:
			void threadAsyncCallback();
		public:
			void addAsync(ActionAsync _elem);
		private:
			zeus::FutureBase callBinary(uint64_t _transactionId,
			                           const ememory::SharedPtr<zeus::Buffer>& _obj,
			                           zeus::FutureData::ObserverFinish _callback=nullptr,
			                           const uint32_t& _service=0);
		public: // section call direct
			template<class... _ARGS>
			zeus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				uint16_t id = getId();
				ememory::SharedPtr<zeus::Buffer> callElem = zeus::createBinaryCall(id, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem);
			}
			template<class... _ARGS>
			zeus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args, zeus::FutureData::ObserverFinish _callback) {
				uint16_t id = getId();
				ememory::SharedPtr<zeus::Buffer> callElem = zeus::createBinaryCall(id, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem, _callback);
			}
		public: // section call with service ID / Client ID
			
			template<class... _ARGS>
			zeus::FutureBase callService(uint32_t _serviceId, const std::string& _functionName, _ARGS&&... _args) {
				uint16_t id = getId();
				ememory::SharedPtr<zeus::Buffer> callElem = zeus::createBinaryCallService(id, _serviceId, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem);
			}
			template<class... _ARGS>
			zeus::FutureBase callServiceAction(uint32_t _serviceId, const std::string& _functionName, _ARGS&&... _args, zeus::FutureData::ObserverFinish _callback) {
				uint16_t id = getId();
				ememory::SharedPtr<zeus::Buffer> callElem = zeus::createBinaryCallService(id, _serviceId, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem, _callback);
			}
			template<class... _ARGS>
			zeus::FutureBase callClient(uint32_t _clientId,
			                           const std::string& _functionName,
			                           _ARGS&&... _args) {
				return callService(_clientId, _functionName, _args...);
			}
			template<class... _ARGS>
			zeus::FutureBase callClientAction(uint32_t _clientId,
			                                 const std::string& _functionName,
			                                 _ARGS&&... _args,
			                                 zeus::FutureData::ObserverFinish _callback) {
				return callServiceAction(_clientId, _functionName, _args..., _callback);
			}
			zeus::FutureBase callForward(uint32_t _clientId,
			                            const ememory::SharedPtr<zeus::Buffer>& _Buffer,
			                            uint64_t _singleReferenceId,
			                            zeus::FutureData::ObserverFinish _callback);
			void callForwardMultiple(uint32_t _clientId,
			                         const ememory::SharedPtr<zeus::Buffer>& _Buffer,
			                         uint64_t _singleReferenceId);
		public: // answers ...
			
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			template<class ZEUS_ARG>
			void answerValue(uint64_t _clientTransactionId, ZEUS_ARG _value, uint32_t _clientId=0) {
				ememory::SharedPtr<zeus::Buffer> answer = zeus::Buffer::create();
				answer->setType(zeus::Buffer::typeMessage::answer);
				answer->setTransactionId(_clientTransactionId);
				answer->setClientId(_clientId);
				answer->addAnswer(_value);
				writeBinary(answer);
			}
			void answerVoid(uint64_t _clientTransactionId, uint32_t _clientId=0);
			void answerError(uint64_t _clientTransactionId, const std::string& _errorValue, const std::string& _errorComment="", uint32_t _clientId=0);
	};
}


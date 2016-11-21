/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <zeus/Buffer.hpp>
#include <zeus/BufferAnswer.hpp>
#include <enet/WebSocket.hpp>
#include <thread>
#include <ememory/memory.hpp>
#include <zeus/AbstractFunction.hpp>
#include <zeus/FutureBase.hpp>

namespace zeus {
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	ememory::SharedPtr<zeus::BufferCall> createBaseCall(uint64_t _transactionId, const std::string& _functionName, const uint32_t& _serviceId=0);
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	void createParam(int32_t _paramId,
	                 ememory::SharedPtr<zeus::BufferCall> _obj);
	
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	template<class ZEUS_TYPE, class... _ARGS>
	void createParam(int32_t _paramId,
	                 ememory::SharedPtr<zeus::BufferCall> _obj,
	                 const ZEUS_TYPE& _param,
	                 _ARGS&&... _args) {
		_obj->addParameter<ZEUS_TYPE>(_param);
		_paramId++;
		createParam(_paramId, _obj, std::forward<_ARGS>(_args)...);
	}
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	// convert const char in std::string ...
	template<class... _ARGS>
	void createParam(int32_t _paramId,
	                 ememory::SharedPtr<zeus::BufferCall> _obj,
	                 const char* _param,
	                 _ARGS&&... _args) {
		createParam(_paramId, _obj, std::string(_param), std::forward<_ARGS>(_args)...);
	}
	
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	template<class... _ARGS>
	ememory::SharedPtr<zeus::BufferCall> createCall(uint64_t _transactionId, const std::string& _functionName, _ARGS&&... _args) {
		ememory::SharedPtr<zeus::BufferCall> callElem = createBaseCall(_transactionId, _functionName);
		if (callElem == nullptr) {
			return nullptr;
		}
		createParam(0, callElem, std::forward<_ARGS>(_args)...);
		return callElem;
	}
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	template<class... _ARGS>
	ememory::SharedPtr<zeus::BufferCall> createCallService(uint64_t _transactionId, const uint32_t& _serviceName, const std::string& _functionName, _ARGS&&... _args) {
		ememory::SharedPtr<zeus::BufferCall> callElem = createBaseCall(_transactionId, _functionName, _serviceName);
		if (callElem == nullptr) {
			return nullptr;
		}
		createParam(0, callElem, std::forward<_ARGS>(_args)...);
		return callElem;
	}
	
	/**
	 * @brief 
	 */
	class WebServer {
		private:
			enet::WebSocket m_connection;
			uint16_t m_transmissionId;
			uint16_t getId() {
				return m_transmissionId++;
			}
			std::mutex m_pendingCallMutex;
			std::vector<std::pair<uint64_t, zeus::FutureBase>> m_pendingCall;
		public:
			using Observer = std::function<void(ememory::SharedPtr<zeus::Buffer>)>; //!< Define an Observer: function pointer
			Observer m_observerElement;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(ememory::SharedPtr<zeus::Buffer>)) {
				m_observerElement = [=](ememory::SharedPtr<zeus::Buffer> _value){
					(*_class.*_func)(_value);
				};
			}
		public:
			using ObserverRequestUri = std::function<bool(const std::string&)>; //!< Define an Observer on the specific URI requested callback: function pointer (return true if the connection is accepted or not)
		protected:
			ObserverRequestUri m_observerRequestUri;
		public:
			/**
			 * @brief Connect on the URI requested.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 */
			template<class CLASS_TYPE>
			void connectUri(CLASS_TYPE* _class, bool (CLASS_TYPE::*_func)(const std::string&)) {
				m_observerRequestUri = [=](const std::string& _value){
					return (*_class.*_func)(_value);
				};
			}
			void connectUri(WebServer::ObserverRequestUri _func) {
				m_observerRequestUri = _func;
			}
			
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			WebServer();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			WebServer(enet::Tcp _connection, bool _isServer);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			virtual ~WebServer();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void setInterface(enet::Tcp _connection, bool _isServer, const std::string& _userName="");
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void connect(bool _async = false);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void disconnect(bool _inThreadStop = false);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool isActive() const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void setInterfaceName(const std::string& _name);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			int32_t writeBinary(ememory::SharedPtr<zeus::Buffer> _data);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void ping();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool onReceiveUri(const std::string& _uri, const std::vector<std::string>& _protocols);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void onReceiveData(std::vector<uint8_t>& _frame, bool _isBinary);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void newBuffer(ememory::SharedPtr<zeus::Buffer> _buffer);
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			const std::chrono::steady_clock::time_point& getLastTimeReceive() {
				return m_connection.getLastTimeReceive();
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			const std::chrono::steady_clock::time_point& getLastTimeSend() {
				return m_connection.getLastTimeSend();
			}
		private:
			using ActionAsync = std::function<bool(WebServer* _interface)>;
			std::mutex m_threadAsyncMutex;
			std::thread* m_threadAsync;
			bool m_threadAsyncRunning;
			std::vector<ActionAsync> m_threadAsyncList;
			std::vector<ActionAsync> m_threadAsyncList2;
		private:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void threadAsyncCallback();
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void addAsync(ActionAsync _elem);
		private:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			zeus::FutureBase callBinary(uint64_t _transactionId,
			                            ememory::SharedPtr<zeus::Buffer> _obj,
			                            zeus::FutureData::ObserverFinish _callback=nullptr,
			                            const uint32_t& _service=0);
		public: // section call direct
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class... _ARGS>
			zeus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				uint16_t id = getId();
				ememory::SharedPtr<zeus::BufferCall> callElem = zeus::createCall(id, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class... _ARGS>
			zeus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args, zeus::FutureData::ObserverFinish _callback) {
				uint16_t id = getId();
				ememory::SharedPtr<zeus::BufferCall> callElem = zeus::createCall(id, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem, _callback);
			}
		public: // section call with service ID / Client ID
			
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class... _ARGS>
			zeus::FutureBase callService(uint32_t _serviceId, const std::string& _functionName, _ARGS&&... _args) {
				uint16_t id = getId();
				ememory::SharedPtr<zeus::BufferCall> callElem = zeus::createCallService(id, _serviceId, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class... _ARGS>
			zeus::FutureBase callServiceAction(uint32_t _serviceId, const std::string& _functionName, _ARGS&&... _args, zeus::FutureData::ObserverFinish _callback) {
				uint16_t id = getId();
				ememory::SharedPtr<zeus::BufferCall> callElem = zeus::createCallService(id, _serviceId, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem, _callback);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class... _ARGS>
			zeus::FutureBase callClient(uint32_t _clientId,
			                            const std::string& _functionName,
			                            _ARGS&&... _args) {
				return callService(_clientId, _functionName, _args...);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class... _ARGS>
			zeus::FutureBase callClientAction(uint32_t _clientId,
			                                  const std::string& _functionName,
			                                  _ARGS&&... _args,
			                                  zeus::FutureData::ObserverFinish _callback) {
				return callServiceAction(_clientId, _functionName, _args..., _callback);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			zeus::FutureBase callForward(uint32_t _clientId,
			                             ememory::SharedPtr<zeus::Buffer> _Buffer,
			                             uint64_t _singleReferenceId,
			                             zeus::FutureData::ObserverFinish _callback);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void callForwardMultiple(uint32_t _clientId,
			                         ememory::SharedPtr<zeus::Buffer> _Buffer,
			                         uint64_t _singleReferenceId);
		public: // answers ...
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class ZEUS_ARG>
			void answerValue(uint64_t _clientTransactionId, ZEUS_ARG _value, uint32_t _clientId=0) {
				ememory::SharedPtr<zeus::BufferAnswer> answer = zeus::BufferAnswer::create();
				answer->setTransactionId(_clientTransactionId);
				answer->setClientId(_clientId);
				answer->addAnswer(_value);
				writeBinary(answer);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void answerVoid(uint64_t _clientTransactionId, uint32_t _clientId=0);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void answerError(uint64_t _clientTransactionId, const std::string& _errorValue, const std::string& _errorComment="", uint32_t _clientId=0);
	};
}


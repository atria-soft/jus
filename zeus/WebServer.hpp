/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once
#include <zeus/message/Message.hpp>
#include <zeus/message/Answer.hpp>
#include <zeus/message/Event.hpp>
#include <zeus/message/Call.hpp>
#include <zeus/message/Progress.hpp>
#include <enet/WebSocket.hpp>
#include <thread>
#include <ememory/memory.hpp>
#include <zeus/AbstractFunction.hpp>
#include <zeus/FutureBase.hpp>
#include <zeus/WebObj.hpp>
#include <ethread/Pool.hpp>

//#define ZEUS_NO_ID_CLIENT (0xFFFFFFFF)
#define ZEUS_NO_ID_CLIENT (0x00000000)
#define ZEUS_NO_ID_OBJECT (0x00000000)
#define ZEUS_ID_SERVICE_ROOT (0x00000000)
#define ZEUS_ID_GATEWAY (0x00000000)
#define ZEUS_ID_GATEWAY_OBJECT (0x0000)
#define ZEUS_GATEWAY_ADDRESS (0x00000000)


namespace zeus {
	class WebServer;
	class ObjectRemoteBase;
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	ememory::SharedPtr<zeus::message::Call> createBaseCall(const ememory::SharedPtr<zeus::WebServer>& _iface,
	                                                       uint64_t _transactionId,
	                                                       const uint32_t& _source,
	                                                       const uint32_t& _destination,
	                                                       const std::string& _functionName);
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	void createParam(int32_t _paramId,
	                 ememory::SharedPtr<zeus::message::Call> _obj);
	
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	template<class ZEUS_TYPE, class... _ARGS>
	void createParam(int32_t _paramId,
	                 ememory::SharedPtr<zeus::message::Call> _obj,
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
	                 ememory::SharedPtr<zeus::message::Call> _obj,
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
	ememory::SharedPtr<zeus::message::Call> createCall(const ememory::SharedPtr<zeus::WebServer>& _iface,
	                                                   uint64_t _transactionId,
	                                                   const uint32_t& _source,
	                                                   const uint32_t& _destination,
	                                                   const std::string& _functionName,
	                                                   _ARGS&&... _args) {
		ememory::SharedPtr<zeus::message::Call> callElem = createBaseCall(_iface, _transactionId, _source, _destination, _functionName);
		if (callElem == nullptr) {
			return nullptr;
		}
		createParam(0, callElem, std::forward<_ARGS>(_args)...);
		return callElem;
	}
	/**
	 * @brief 
	 */
	class WebServer : public ememory::EnableSharedFromThis<zeus::WebServer> {
		protected:
			std::mutex m_mutex;
		public:
			std::vector<ememory::SharedPtr<zeus::WebObj>> m_actifObject; //!< List of all active object created and that remove is in progress ...
		private:
			enet::WebSocket m_connection;
			ethread::Pool m_processingPool;
			std::vector<ememory::SharedPtr<zeus::Message>> m_listPartialMessage;
			uint16_t m_localAddress;
			uint16_t m_localIdObjectIncrement; //!< attribute an unique ID for an object
		public:
			uint16_t getAddress() const {
				return m_localAddress;
			}
			void setAddress(uint16_t _address) {
				m_localAddress = _address;
			}
			uint16_t getNewObjectId() {
				return m_localIdObjectIncrement++;
			}
		private:
			std::vector<ememory::SharedPtr<zeus::WebObj>> m_listObject;
			std::vector<ememory::WeakPtr<zeus::ObjectRemoteBase>> m_listRemoteObject;
		public:
			void addWebObj(ememory::SharedPtr<zeus::WebObj> _obj);
			void addWebObjRemote(ememory::SharedPtr<zeus::ObjectRemoteBase> _obj);
			void cleanDeadObject();
			/**
			 * @brief Set the list of interface that has been removed ...
			 */
			void interfaceRemoved(std::vector<uint16_t> _list);
		private:
			uint32_t m_interfaceId;
			uint16_t m_transmissionId;
			uint16_t getId() {
				return m_transmissionId++;
			}
			std::mutex m_pendingCallMutex;
			std::vector<std::pair<uint64_t, zeus::FutureBase>> m_pendingCall;
		public:
			using Observer = std::function<void(ememory::SharedPtr<zeus::Message>)>; //!< Define an Observer: function pointer
			Observer m_observerElement;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(ememory::SharedPtr<zeus::Message>)) {
				m_observerElement = [=](ememory::SharedPtr<zeus::Message> _value){
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
			WebServer(); // TODO : Set it in a factory to force the use of sharedPtr
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
			int32_t writeBinary(ememory::SharedPtr<zeus::Message> _data);
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
			void newMessage(ememory::SharedPtr<zeus::Message> _buffer);
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			const echrono::Steady& getLastTimeReceive() {
				return m_connection.getLastTimeReceive();
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			const echrono::Steady& getLastTimeSend() {
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
			                            ememory::SharedPtr<zeus::Message> _obj,
			                            const uint32_t& _service=0);
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class... _ARGS>
			zeus::FutureBase call(const uint32_t& _source, const uint32_t& _destination, const std::string& _functionName, _ARGS&&... _args) {
				uint16_t id = getId();
				ememory::SharedPtr<zeus::message::Call> callElem = zeus::createCall(sharedFromThis(), id, _source, _destination, _functionName, std::forward<_ARGS>(_args)...);
				return callBinary(id, callElem);
			}
		public: // Events ...
			/**
			 * @brief Send a progression value to a specific call in progress
			 * @param[in] _transactionId Current trasaction ID
			 * @param[in] _source Source Id of the sending value
			 * @param[in] _destination Destinatio of the progression
			 * @param[in] _value Value to send
			 */
			template<class ZEUS_ARG>
			void eventValue(uint32_t _clientTransactionId, uint32_t _source, uint32_t _destination, ZEUS_ARG _value) {
				ememory::SharedPtr<zeus::message::Event> event = zeus::message::Event::create(sharedFromThis());
				event->setTransactionId(_clientTransactionId);
				event->setSource(_source);
				event->setDestination(_destination);
				event->addEvent(_value);
				writeBinary(event);
			}
		public: // answers ...
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			/**
			 * @brief Send an Answer of a function with single value
			 * @param[in] _clientTransactionId Transaction ID
			 * @param[in] _value ... Value to return
			 * @param[in] _srcObjectId Client to send control
			 */
			template<class ZEUS_ARG>
			void answerValue(uint32_t _clientTransactionId, uint32_t _source, uint32_t _destination, ZEUS_ARG _value) {
				ememory::SharedPtr<zeus::message::Answer> answer = zeus::message::Answer::create(sharedFromThis());
				answer->setTransactionId(_clientTransactionId);
				answer->setSource(_source);
				answer->setDestination(_destination);
				answer->addAnswer(_value);
				writeBinary(answer);
			}
			/**
			 * @brief Send an Answer value (no value to set ==> void return of function)
			 * @param[in] _clientTransactionId Transaction ID
			 * @param[in] _srcObjectId Client to send control
			 */
			void answerVoid(uint32_t _clientTransactionId, uint32_t _source, uint32_t _destination);
			/**
			 * @brief Send an Answer error of a function
			 * @param[in] _clientTransactionId Transaction ID
			 * @param[in] _errorValue Value of the error
			 * @param[in] _errorComment Help comment of the error
			 * @param[in] _srcObjectId Client to send control
			 */
			void answerError(uint32_t _clientTransactionId, uint32_t _source, uint32_t _destination, const std::string& _errorValue, const std::string& _errorComment="");
		public:
			// for debug only:
			void listObjects();
			bool transferRemoteObjectOwnership(uint16_t _objectAddress, uint32_t _sourceAddress, uint32_t _destinataireAddress);
			bool removeObjectOwnership(uint16_t _objectAddress, uint32_t _sourceAddress);
	};
}


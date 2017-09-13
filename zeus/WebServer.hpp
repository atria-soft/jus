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
#include <enet/WebSocket.hpp>
#include <ethread/Thread.hpp>
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
	 * @brief Create a basic call message with all the basic information needed
	 * @param[in] _iface WebServer interface handle
	 * @param[in] _transactionId UniqueId of the transaction (must be != 0)
	 * @param[in] _source Unique local zeus network adress of a service or an Object (the source of the message address)
	 * @param[in] _destination Unique local zeus network adress of a service or an Object (the destination of the message address)
	 * @param[in] _functionName The name of the function to call
	 * @return handle on the new message to call on the remote object
	 */
	ememory::SharedPtr<zeus::message::Call> createBaseCall(const ememory::SharedPtr<zeus::WebServer>& _iface,
	                                                       uint64_t _transactionId,
	                                                       const uint32_t& _source,
	                                                       const uint32_t& _destination,
	                                                       const etk::String& _functionName);
	/**
	 * @brief This is the last call of createParam recursive function (no more parameter to add)
	 * @param[in] _parmaId Id of the parameter to add.
	 * @param[in] _obj message where to add the parameter.
	 */
	void createParam(int32_t _paramId,
	                 ememory::SharedPtr<zeus::message::Call> _obj);
	
	/**
	 * @brief Template to add a parameter of a function in recursive form
	 * @param[in] _parmaId Id of the parameter to add.
	 * @param[in] _obj message where to add the parameter.
	 * @param[in] _param Parameter value to add.
	 * @param[in] _args... other argument to add (in recursive call)
	 */
	template<class ZEUS_TYPE, class... _ARGS>
	void createParam(int32_t _paramId,
	                 ememory::SharedPtr<zeus::message::Call> _obj,
	                 const ZEUS_TYPE& _param,
	                 _ARGS&&... _args) {
		_obj->addParameter<ZEUS_TYPE>(_param);
		_paramId++;
		createParam(_paramId, _obj, etk::forward<_ARGS>(_args)...);
	}
	/**
	 * @brief Template specialization in 'const char*' to add a parameter of a function in recursive form
	 * @param[in] _parmaId Id of the parameter to add.
	 * @param[in] _obj message where to add the parameter.
	 * @param[in] _param Parameter value to add (char* that is converted in etk::String).
	 * @param[in] _args... other argument to add (in recursive call)
	 */
	template<class... _ARGS>
	void createParam(int32_t _paramId,
	                 ememory::SharedPtr<zeus::message::Call> _obj,
	                 const char* _param,
	                 _ARGS&&... _args) {
		createParam(_paramId, _obj, etk::String(_param), etk::forward<_ARGS>(_args)...);
	}
	/**
	 * @brieftemplate to create a ZEUS CALL message with all the parameter in arguments
	 * @param[in] _iface WebServer interface handle
	 * @param[in] _transactionId UniqueId of the transaction (must be != 0)
	 * @param[in] _source Unique local zeus network adress of a service or an Object (the source of the message address)
	 * @param[in] _destination Unique local zeus network adress of a service or an Object (the destination of the message address)
	 * @param[in] _functionName The name of the function to call
	 * @param[in] _args... argument of the call to do
	 * @return handle on the new message to call on the remote object
	 */
	template<class... _ARGS>
	ememory::SharedPtr<zeus::message::Call> createCall(const ememory::SharedPtr<zeus::WebServer>& _iface,
	                                                   uint64_t _transactionId,
	                                                   const uint32_t& _source,
	                                                   const uint32_t& _destination,
	                                                   const etk::String& _functionName,
	                                                   _ARGS&&... _args) {
		ememory::SharedPtr<zeus::message::Call> callElem = createBaseCall(_iface, _transactionId, _source, _destination, _functionName);
		if (callElem == nullptr) {
			return nullptr;
		}
		createParam(0, callElem, etk::forward<_ARGS>(_args)...);
		return callElem;
	}
	/**
	 * @brief Web interface of a service engine
	 */
	class WebServer : public ememory::EnableSharedFromThis<zeus::WebServer> {
		protected:
			ethread::Mutex m_mutex; //!< main interface lock
		public:
			etk::Vector<ememory::SharedPtr<zeus::WebObj>> m_actifObject; //!< List of all active object created and that remove is in progress ...
		private:
			enet::WebSocket m_connection; //!< Zeus protocol is based on a webSocket to be compatible with Java-script
			ethread::Pool m_processingPool; //!< Thread pool processing of the input data
			etk::Vector<ememory::SharedPtr<zeus::Message>> m_listPartialMessage; //!< list of all message that data has not finished to arrive.
			uint16_t m_localAddress; //!< Local client address.
			uint16_t m_localIdObjectIncrement; //!< attribute an unique ID for an object.
		public:
			/**
			 * @brief Get the unique ID of the client
			 * @return the client Mai address
			 */
			uint16_t getAddress() const {
				return m_localAddress;
			}
			/**
			 * @brief Set the new wlient address
			 * @param[in] _address Address of the client
			 */
			void setAddress(uint16_t _address) {
				m_localAddress = _address;
			}
			/**
			 * @brief Get a new unique object ID
			 * @return a new single object ID
			 */
			uint16_t getNewObjectId() {
				ethread::UniqueLock lock(m_mutex);
				return m_localIdObjectIncrement++;
			}
		private:
			etk::Vector<ememory::SharedPtr<zeus::WebObj>> m_listObject; //!< List of all local object that is reference in the system.
			etk::Vector<ememory::WeakPtr<zeus::ObjectRemoteBase>> m_listRemoteObject; //!< List of all object that we have a reference in the local interface.
		public:
			/**
			 * @brief Add a local WebObject to maage all his callback
			 * @param[in] _obj Object to add
			 */
			void addWebObj(ememory::SharedPtr<zeus::WebObj> _obj);
			/**
			 * @brief Add a REMOTE WebObject to maage all his callback
			 * @param[in] _obj Object to add
			 */
			void addWebObjRemote(ememory::SharedPtr<zeus::ObjectRemoteBase> _obj);
			/**
			 * @brief Clear all Object that have no more remote user.
			 */
			void cleanDeadObject();
			/**
			 * @brief Set the list of interface that has been removed ...
			 */
			void interfaceRemoved(etk::Vector<uint16_t> _list);
		private:
			uint32_t m_interfaceId; //!< local client interface ID
			uint16_t m_transmissionId; //!< Unique Id of a transmission (it is != 0)
			/**
			 * @brief Get a new transmission ID
			 * @return Unique ID of the transmision
			 */
			uint16_t getId();
			ethread::Mutex m_pendingCallMutex; //!< local call of a pendinc call venctor update
			etk::Vector<etk::Pair<uint64_t, zeus::FutureBase>> m_pendingCall; //!< List of pending call interface
		public:
			using Observer = etk::Function<void(ememory::SharedPtr<zeus::Message>)>; //!< Define an Observer: function pointer
			Observer m_observerElement; //!< Observer on a new message arriving
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
			using ObserverRequestUri = etk::Function<bool(const etk::String&)>; //!< Define an Observer on the specific URI requested callback: function pointer (return true if the connection is accepted or not)
		protected:
			ObserverRequestUri m_observerRequestUri; //!< Observer on a requesting URI connection
		public:
			/**
			 * @brief Connect on the URI requested.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 */
			template<class CLASS_TYPE>
			void connectUri(CLASS_TYPE* _class, bool (CLASS_TYPE::*_func)(const etk::String&)) {
				m_observerRequestUri = [=](const etk::String& _value){
					return (*_class.*_func)(_value);
				};
			}
			/**
			 * @brief Set the Observer on URI requesting
			 * @param[in] _func Observer function
			 */
			void connectUri(WebServer::ObserverRequestUri _func) {
				m_observerRequestUri = _func;
			}
		public:
			/**
			 * @brief Webserver contructor
			 */
			WebServer(); // TODO : Set it in a factory to force the use of sharedPtr
			/**
			 * @brief Webserver contructor
			 * @param[in] _connection TCP connection interface
			 * @param[in] _isServer Set it true if the local interface in a server
			 */
			WebServer(enet::Tcp _connection, bool _isServer);
			/**
			 * @brief Destructor
			 */
			virtual ~WebServer();
			/**
			 * @brief Set the low level network interface
			 * @param[in] _connection TCP connection interface
			 * @param[in] _isServer Set it true if the local interface in a server
			 * @param[in] _userName Name on the user connected
			 * @return 
			 */
			void setInterface(enet::Tcp _connection, bool _isServer, const etk::String& _userName="");
			/**
			 * @brief Connect on the remote interface
			 * @param[in] _async if true, the cunnection does not wait all the connection process is done to return
			 */
			void connect(bool _async = false);
			/**
			 * @brief Disconnect from the remote
			 * @param[in] _inThreadStop The call is done in the thread processing the input message
			 */
			void disconnect(bool _inThreadStop = false);
			/**
			 * @brief Check if the link is alive
			 * @return true The connection is alive
			 * @return false THe connection is dead
			 */
			bool isActive() const;
			/**
			 * @brief set the interface name
			 * @param[in] _name Ne name of the interface
			 */
			void setInterfaceName(const etk::String& _name);
			/**
			 * @brief Write a message preformated
			 * @param[in] _data Message to write
			 * @return Number of byte written
			 */
			int32_t writeBinary(ememory::SharedPtr<zeus::Message> _data);
			/**
			 * @brief Send a ping to the remote connection
			 */
			void ping();
			/**
			 * @brief Called by the underprotocol for a new URI connection
			 * @param[in] _uri URI connection (GET)
			 * @param[in] _protocols Protocol requested in the URI (ZEUS:1.0/ZEUS:0.8)
			 * @return true the connection is accepted
			 * @return false the connection is rejected
			 */
			bool onReceiveUri(const etk::String& _uri, const etk::Vector<etk::String>& _protocols);
			/**
			 * @brief The Zeus protocol is based on a webSocket, then the connection can send full fragment (it call newMessage when data is parsed
			 * @param[in] _frame A frame that has been just received
			 * @param[in] _isBinary the frame is binary if true, it is a string id false
			 */
			void onReceiveData(etk::Vector<uint8_t>& _frame, bool _isBinary);
			/**
			 * @brief Receive a message to parse
			 * @param[in] _buffer Message to interprete
			 */
			void newMessage(ememory::SharedPtr<zeus::Message> _buffer);
		public:
			/**
			 * @brief Get the last time a transmission has been done on the websocket (RECEIVE)
			 * @return Steady Time of the last transmission
			 */
			const echrono::Steady& getLastTimeReceive() {
				return m_connection.getLastTimeReceive();
			}
			/**
			 * @brief Get the last time a transmission has been done on the websocket (EMIT)
			 * @return Steady Time of the last transmission
			 */
			const echrono::Steady& getLastTimeSend() {
				return m_connection.getLastTimeSend();
			}
		private:
			using ActionAsync = etk::Function<bool(WebServer* _interface)>; //!< type of the action for sending big data on the websocket
			ethread::Mutex m_threadAsyncMutex; //!< Mutex fot the thread to send async data
			ethread::Thread* m_threadAsync; //!< sending async data thread. TODO: Set it in a thread pool ...
			bool m_threadAsyncRunning; //!< Threa is running
			etk::Vector<ActionAsync> m_threadAsyncList; //!< List of action to send (current)
			etk::Vector<ActionAsync> m_threadAsyncList2; //!< list of action to send whenwurrent is sending in progress
			// TODO: Abord async sender ...
		private:
			/**
			 * @brief Thread callback to send data
			 */
			void threadAsyncCallback();
		public:
			/**
			 * @brief Add an async data to send
			 * @param[in] _elem Action lambda to send data
			 */
			void addAsync(ActionAsync _elem);
		private:
			/**
			 * @brief Add a message in the list of data that might be sended
			 * @param[in] _obj Message to send
			 * @return Future that will get the return values
			 */
			zeus::FutureBase callBinary(ememory::SharedPtr<zeus::Message> _obj);
		public:
			/**
			 * @brief Create a call with specific parameteurs
			 * @param[in] _source Source Id of the sending value
			 * @param[in] _destination Destinatio of the progression
			 * @param[in] _functionName Function name to call
			 * @param[in] _args... Argument to add on the current message
			 * @return Future that will get the return values
			 */
			template<class... _ARGS>
			zeus::FutureBase call(const uint32_t& _source, const uint32_t& _destination, const etk::String& _functionName, _ARGS&&... _args) {
				uint16_t id = getId();
				ememory::SharedPtr<zeus::message::Call> callElem = zeus::createCall(sharedFromThis(), id, _source, _destination, _functionName, etk::forward<_ARGS>(_args)...);
				return callBinary(callElem);
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
			 * @brief Create an ansers with PROTOCLL error and an help
			 * @param[in] _transactionId Current trasaction ID
			 * @param[in] _errorHelp Help for the user to understand the error and correct it
			 */
			void answerProtocolError(uint32_t _transactionId, const etk::String& _errorHelp);
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
			void answerError(uint32_t _clientTransactionId, uint32_t _source, uint32_t _destination, const etk::String& _errorValue, const etk::String& _errorComment="");
		public:
			/**
			 * @brief Display list of all objects
			 * @note For debug only
			 */
			void listObjects();
			/**
			 * @brief Transfer the remote object onership from an adress to an other
			 * @param[in] _objectAddress Local interface object address (uID)
			 * @param[in] _sourceAddress Remote object to change the address
			 * @param[in] _destinataireAddress New adress that replace the _sourceAddress
			 * @return true The onership has been changed
			 * @return false An error apears
			 */
			bool transferRemoteObjectOwnership(uint16_t _objectAddress, uint32_t _sourceAddress, uint32_t _destinataireAddress);
			/**
			 * @brief Remove a local object dependence on a remote object ==> permit to the server to remove it
			 * @param[in] _objectAddress Local interface object address (uID)
			 * @param[in] _sourceAddress Remote object that the adress must be removed
			 * @return true The onership has been removed
			 * @return false An error apears
			 */
			bool removeObjectOwnership(uint16_t _objectAddress, uint32_t _sourceAddress);
	};
}


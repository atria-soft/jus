/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <eproperty/Value.hpp>
#include <zeus/debug.hpp>
#include <chrono>

#include <zeus/Future.hpp>
#include <zeus/ObjectRemote.hpp>
#include <zeus/Object.hpp>

namespace zeus {
	class Object;
	/**
	 * @brief Client interface to acces on the remote service and gateway
	 */
	class Client : public eproperty::Interface {
		friend class ObjectRemote;
		public:
			eproperty::Value<etk::String> propertyIp; //!< Ip of WebSocket TCP connection
			eproperty::Value<uint16_t> propertyPort; //!< Port of the WebSocket connection
		public:
			etk::String m_clientName; //!< Local client name to generate the local serrvice name if needed (if direct connection ==> no name)
			ememory::SharedPtr<zeus::WebServer> m_interfaceWeb; //!< Interface on the Websocket interface
			etk::Vector<ememory::WeakPtr<zeus::ObjectRemoteBase>> m_listConnectedService; //!< Connect only one time on each service, not needed more.
		public:
			/**
			 * @brief answer a protocol error on the websocket ==> this stop the communication
			 * @param[in] _transactionId The tansation ID that have an error
			 * @param[in] _errorHelp Help developper/user to understand where the problem come from.
			 */
			void answerProtocolError(uint32_t _transactionId, const etk::String& _errorHelp);
			/**
			 * @brief Get the client web interface
			 * @return A shared pointer on the client server
			 */
			ememory::SharedPtr<zeus::WebServer> getWebInterface() {
				return m_interfaceWeb;
			}
			/**
			 * @brief Contructor of a client
			 */
			Client();
			/**
			 * @brief Destructor of a client
			 */
			virtual ~Client();
		protected:
			/**
			 * @brief Connect to a remote extern server
			 * @param[in] _address Address of the user: "ABCD.efgh~atria-soft.com:1993"
			 * @param[in] _timeOut duration that we are waiting the server answer
			 */
			bool connectTo(const etk::String& _address, echrono::Duration _timeOut = echrono::seconds(1));
		public:
			/**
			 * @brief Direct connection on a GateWay No Identification needed, the Port on the Gateway garenty the connection)
			 * @note This is exclusively reserve for server service provicers
			 * @param[in] _timeOut duration that we are waiting the server answer
			 */
			bool connect(echrono::Duration _timeOut = echrono::seconds(1));
			/**
			 * @brief Create a client on a specific user in an ANONIMOUS way
			 * @param[in] _address Address of the user: "ABCD.efgh~atria-soft.com:1993"
			 * @param[in] _timeOut duration that we are waiting the server answer
			 */
			bool connect(const etk::String& _address, echrono::Duration _timeOut = echrono::seconds(1));
			/**
			 * @brief Create a client on a specific user in a user mode (connect to your personnal account)
			 * @param[in] _address Address of the user: "ABCD.efgh~atria-soft.com:1993"
			 * @param[in] _userPassword Password of the user
			 * @param[in] _timeOut duration that we are waiting the server answer
			 */
			bool connect(const etk::String& _address, const etk::String& _userPassword, echrono::Duration _timeOut = echrono::seconds(1));
			/**
			 * @brief Create a client on a specific user in a client mode with the tocken associated
			 * @param[in] _address Address of the user: "ABCD.efgh~atria-soft.com:1993"
			 * @param[in] 
			 * @param[in] 
			 * @param[in] _timeOut duration that we are waiting the server answer
			 */
			bool connect(const etk::String& _address, const etk::String& _clientName, const etk::String& _clientTocken, echrono::Duration _timeOut = echrono::seconds(1));
			/**
			 * @brief Disconnect of the current interface
			 */
			void disconnect();
		public:
			/**
			 * @brief Get a remote service
			 * @param[in] _serviceName Name of the service
			 * @return Pointer on an interface of remote service
			 */
			zeus::ObjectRemote getService(const etk::String& _serviceName);
			
			using factoryService = std::function<void(uint32_t, ememory::SharedPtr<zeus::WebServer>& _iface, uint32_t _destination)>; //!< call this function anser to the caller the requested Object
			
			etk::Map<etk::String,factoryService> m_listServicesAvaillable; //!< list of all factory availlable (to create new services)
			/**
			 * @brief Provide a service with a specific name
			 * @param[in] _serviceName Name of the service
			 * @param[in] _service handle on the service provided
			 * @return true if the service is acepted or false if not
			 */
			bool serviceAdd(const etk::String& _serviceName, factoryService _factory);
			/**
			 * @brief Revmove a service from the list of availlable services
			 * @param[in] _serviceName Name of the service to remove
			 * @return true The service has been removed, false otherwise.
			 */
			bool serviceRemove(const etk::String& _serviceName);
		private:
			/**
			 * @brief When receive data from the websocket ... call this ...
			 * @param[in] _value New input buffer
			 */
			void onClientData(ememory::SharedPtr<zeus::Message> _value);
		public:
			/**
			 * @brief Create a call on the interface gateway (threw the router)
			 * @param[in] _functionName name of the function to call
			 * @param[in] _args... multiple argument neededs
			 * @return a future that will contain the aswer when receiveed (need to transmit over ethernet)
			 */
			template<class... _ARGS>
			zeus::FutureBase call(uint16_t _srcObjectId,
			                      uint32_t _destination,
			                      const etk::String& _functionName,
			                      _ARGS&&... _args) {
				if (m_interfaceWeb == nullptr) {
					ememory::SharedPtr<zeus::message::Answer> ret = zeus::message::Answer::create(nullptr); // TODO : This is really a bad case ...
					ret->addError("NULLPTR", "call " + _functionName + " with no interface open");
					return zeus::FutureBase(0, ret);
				}
				uint32_t source = (uint32_t(m_interfaceWeb->getAddress()) << 16) + _srcObjectId;
				return m_interfaceWeb->call(source, _destination, _functionName, _args...);
			}
			uint16_t getlocalAddress() {
				return m_interfaceWeb->getAddress();
			}
		private:
			/**
			 * @brief Internal (called when user change the Ip of the client interface)
			 */
			void onPropertyChangeIp();
			/**
			 * @brief  Internal (called when user change the port of the client interface)
			 */
			void onPropertyChangePort();
		public:
			/**
			 * @brief Send a ping to prevent the protocol time-out (no transmission)
			 * @note let the system use this function.
			 */
			void pingIsAlive();
			/**
			 * @brief Check if the server/connection is alive
			 * @return true The connection is alive
			 * @return false The connection is dead
			 */
			bool isAlive();
			/**
			 * @brief Display all connected object remote and local ...
			 */
			void displayConnectedObject();
			/**
			 * @brief Display all connected object remote and local ...
			 */
			void cleanDeadObject();
		public:
			// TODO: Remove this from here ... ==> create a proxy to gateway (service manager)
			/**
			 * @brief Get the number of services.
			 * @return Future on the services count.
			 */
			zeus::Future<int32_t> getServiceCount();
			/**
			 * @brief Get the whole list of services availlable
			 * @return Future on the list of service (names)
			 */
			zeus::Future<etk::Vector<etk::String>> getServiceList();
			// TODO : This is an active waiting ==> this is bad ... ==> use future, it will be better
			/**
			 * @brief Wait for a service wake up (and be availlable)
			 * @param[in] _serviceName Name of the service to wait.
			 * @param[in] _delta Duration to wait the service
			 * @return true The service is availlable
			 * @return false The service is not availlable.
			 */
			bool waitForService(const etk::String& _serviceName, echrono::Duration _delta = echrono::seconds(1));
	};
}


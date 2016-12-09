/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
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
			eproperty::Value<std::string> propertyIp; //!< Ip of WebSocket TCP connection
			eproperty::Value<uint16_t> propertyPort; //!< Port of the WebSocket connection
		public:
			std::string m_clientName; //!< Local client name to generate the local serrvice name if needed (if direct connection ==> no name)
			ememory::SharedPtr<zeus::WebServer> m_interfaceWeb; //!< Interface on the Websocket interface
			std::vector<ememory::WeakPtr<zeus::ObjectRemoteBase>> m_listConnectedService; //!< Connect only one time on each service, not needed more.
		public:
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			ememory::SharedPtr<zeus::WebServer> getWebInterface() {
				return m_interfaceWeb;
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			Client();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			virtual ~Client();
		protected:
			/**
			 * @brief Connetc to a remote extern server
			 * @param[in] _address Address of the user: "ABCD.efgh~atria-soft.com:1993"
			 */
			bool connectTo(const std::string& _address);
		public:
			/**
			 * @brief Direct connection on a GateWay No Identification needed, the Port on the Gateway garenty the connection)
			 * @note This is exclusively reserve for server service provicers
			 */
			bool connect();
			/**
			 * @brief Create a client on a specific user in an ANONIMOUS way
			 * @param[in] _address Address of the user: "ABCD.efgh~atria-soft.com:1993"
			 */
			bool connect(const std::string& _address);
			/**
			 * @brief Create a client on a specific user in a user mode (connect to your personnal account)
			 * @param[in] _address Address of the user: "ABCD.efgh~atria-soft.com:1993"
			 * @param[in] _userPassword Password of the user
			 */
			bool connect(const std::string& _address, const std::string& _userPassword);
			/**
			 * @brief Create a client on a specific user in a client mode with the tocken associated
			 * @param[in] _address Address of the user: "ABCD.efgh~atria-soft.com:1993"
			 * @param[in] 
			 * @param[in] 
			 */
			bool connect(const std::string& _address, const std::string& _clientName, const std::string& _clientTocken);
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
			zeus::ObjectRemote getService(const std::string& _serviceName);
			using factoryService = std::function<void(uint32_t, ememory::SharedPtr<zeus::WebServer>& _iface, uint32_t _destination)>; // call this function anser to the callter the requested Object
			
			std::map<std::string,factoryService> m_listServicesAvaillable; //!< list of all factory availlable
			/**
			 * @brief Provide a service with a specific name
			 * @param[in] _serviceName Name of the service
			 * @param[in] _service handle on the service provided
			 * @return true if the service is acepted or false if not
			 */
			bool serviceAdd(const std::string& _serviceName, factoryService _factory);
			bool serviceRemove(const std::string& _serviceName);
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
			                      const std::string& _functionName,
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
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void pingIsAlive();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
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
	};
}


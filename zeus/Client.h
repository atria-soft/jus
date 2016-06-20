/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/TcpString.h>
#include <eproperty/Value.h>
#include <zeus/debug.h>
#include <chrono>
#include <unistd.h>
#include <zeus/Future.h>
#include <zeus/ServiceRemote.h>

namespace zeus {
	class Client : public eproperty::Interface {
		friend class ServiceRemote;
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
		private:
			ememory::SharedPtr<zeus::TcpString> m_interfaceClient;
		public:
			/**
			 * @brief Create a client on a specific user in a client mode with the tocken associated
			 * @param[in] _address Address of the user: "ABCD.efgh#atria-soft.com:1993"
			 * @param[in] 
			 * @param[in] 
			 */
			//Client(const std::string& _address, const std::string& _clientName, const std::string& _clientTocken);
			/**
			 * @brief Create a client on a specific user in a user mode (connect to your personnal account)
			 * @param[in] _address Address of the user: "ABCD.efgh#atria-soft.com:1993"
			 * @param[in] _userPassword Password of the user
			 */
			//Client(const std::string& _address, const std::string& _userPassword);
			/**
			 * @brief Create a client on a specific user in an ANONIMOUS way
			 * @param[in] _address Address of the user: "ABCD.efgh#atria-soft.com:1993"
			 */
			//Client(const std::string& _address);
			
			Client();
			virtual ~Client();
			bool connect(const std::string& _remoteUserToConnect);
			void disconnect();
		public:
			zeus::ServiceRemote getService(const std::string& _serviceName);
			
			// Connect that is not us
			//bool identify("clientTest1#atria-soft.com", "QSDQSDGQSF54HSXWVCSQDJ654URTDJ654NBXCDFDGAEZ51968");
			// Connect to ourself:
			//client1.authentificate("coucou");
		private:
			void onClientData(zeus::Buffer& _value);
		public:
			template<class... _ARGS>
			zeus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				if (m_interfaceClient == nullptr) {
					zeus::Buffer ret;
					ret.addError("NULLPTR", "call " + _functionName + " with no interface open");
					return zeus::FutureBase(0, true, ret);
				}
				return m_interfaceClient->call(_functionName, _args...);
			}
			template<class... _ARGS>
			zeus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args, zeus::FutureData::ObserverFinish _callback) {
				if (m_interfaceClient == nullptr) {
					zeus::Buffer ret;
					ret.addError("NULLPTR", "call " + _functionName + " with no interface open");
					return zeus::FutureBase(0, true, ret, _callback);
				}
				return m_interfaceClient->callAction(_functionName, _args..., _callback);
			}
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
	};
}


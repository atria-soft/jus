/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <eproperty/Value.h>
#include <jus/debug.h>
#include <chrono>
#include <unistd.h>
#include <jus/Future.h>
#include <jus/connectionMode.h>
#include <jus/ServiceRemote.h>

namespace jus {
	class Client : public eproperty::Interface {
		friend class ServiceRemote;
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
		private:
			ememory::SharedPtr<jus::TcpString> m_interfaceClient;
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
			jus::ServiceRemote getService(const std::string& _serviceName);
			
			// Connect that is not us
			//bool identify("clientTest1#atria-soft.com", "QSDQSDGQSF54HSXWVCSQDJ654URTDJ654NBXCDFDGAEZ51968");
			// Connect to ourself:
			//client1.authentificate("coucou");
		private:
			void onClientData(jus::Buffer& _value);
		public:
			template<class... _ARGS>
			jus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				if (m_interfaceClient == nullptr) {
					jus::Buffer ret;
					ret.addError("NULLPTR", "call " + _functionName + " with no interface open");
					return jus::FutureBase(0, true, ret);
				}
				return m_interfaceClient->call(_functionName, _args...);
			}
			template<class... _ARGS>
			jus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args, jus::FutureData::ObserverFinish _callback) {
				if (m_interfaceClient == nullptr) {
					jus::Buffer ret;
					ret.addError("NULLPTR", "call " + _functionName + " with no interface open");
					return jus::FutureBase(0, true, ret, _callback);
				}
				return m_interfaceClient->callAction(_functionName, _args..., _callback);
			}
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
	};
}


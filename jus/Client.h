/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <eproperty/Value.h>
#include <ejson/ejson.h>
#include <jus/debug.h>
#include <jus/ServiceRemote.h>
#include <chrono>
#include <unistd.h>
#include <jus/Future.h>
#include <jus/connectionMode.h>

namespace jus {
	class Client : public eproperty::Interface {
		friend class ServiceRemote;
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
			std::mutex m_mutex;
			std::vector<jus::FutureBase> m_pendingCall;
		protected:
			enum jus::connectionMode m_interfaceMode;
		public:
			enum jus::connectionMode getMode() { return m_interfaceMode; }
		private:
			jus::TcpString m_interfaceClient;
			uint32_t m_id;
			std::vector<std::string> m_newData;
		public:
			Client();
			virtual ~Client();
			bool connect(const std::string& _remoteUserToConnect);
			void disconnect();
		public:
			jus::ServiceRemote getService(const std::string& _serviceName);
			int32_t link(const std::string& _serviceName);
			bool unlink(const uint32_t& _serviceId);
			
			// Connect that is not us
			//bool identify("clientTest1#atria-soft.com", "QSDQSDGQSF54HSXWVCSQDJ654URTDJ654NBXCDFDGAEZ51968");
			// Connect to ourself:
			//client1.authentificate("coucou");
		private:
			void onClientData(std::string _value);
			std::string asyncRead();
			jus::FutureBase callJson(uint64_t _transactionId,
			                         ejson::Object _obj,
			                         const std::vector<ActionAsyncClient>& _async,
			                         jus::FutureData::ObserverFinish _callback=nullptr,
			                         const uint32_t& _service=0);
			jus::FutureBase callBinary(uint64_t _transactionId,
			                           jus::Buffer& _obj,
			                           const std::vector<ActionAsyncClient>& _async,
			                           jus::FutureData::ObserverFinish _callback=nullptr,
			                           const uint32_t& _service=0);
		public:
			uint64_t getId();
			template<class... _ARGS>
			jus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				uint64_t id = getId();
				std::vector<ActionAsyncClient> asyncAction;
				if (getMode() == jus::connectionMode::modeJson) {
					ejson::Object callElem = jus::createCall(asyncAction, id, _functionName, std::forward<_ARGS>(_args)...);
					return callJson(id, callElem, asyncAction);
				} else {
					jus::Buffer callElem = jus::createBinaryCall(asyncAction, id, _functionName, std::forward<_ARGS>(_args)...);
					return callBinary(id, callElem, asyncAction);
				}
			}
			template<class... _ARGS>
			jus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args, jus::FutureData::ObserverFinish _callback) {
				uint64_t id = getId();
				std::vector<ActionAsyncClient> asyncAction;
				if (getMode() == jus::connectionMode::modeJson) {
					ejson::Object callElem = jus::createCall(asyncAction, id, _functionName, std::forward<_ARGS>(_args)...);
					return callJson(id, callElem, asyncAction, _callback);
				} else {
					jus::Buffer callElem = jus::createBinaryCall(asyncAction, id, _functionName, std::forward<_ARGS>(_args)...);
					return callBinary(id, callElem, asyncAction, _callback);
				}
			}
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
	};
}


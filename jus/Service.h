/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <eproperty/Value.h>
#include <ejson/ejson.h>
#include <jus/AbstractFunctionTypeDirect.h>
#include <jus/AbstractFunctionTypeClass.h>
#include <jus/debug.h>

namespace jus {
	class Service : public eproperty::Interface {
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
		private:
			jus::TcpString m_interfaceClient;
			uint32_t m_id;
			esignal::Connection m_dataCallback;
			std::vector<std::string> m_newData;
		public:
			Service();
			virtual ~Service();
			// Genenric function call:
			ejson::Object callJson(const ejson::Object& _obj);
			virtual ejson::Object callJson2(size_t _clientId, const ejson::Object& _obj) = 0;
			void connect(const std::string& _serviceName);
			void disconnect();
		private:
			void onClientData(const std::string& _value);
			std::string asyncRead();
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
		public:
			template<class JUS_RETURN_VALUE,
			         class... JUS_FUNC_ARGS_TYPE>
			void advertise(const std::string& _name,
			               JUS_RETURN_VALUE (*_func)(JUS_FUNC_ARGS_TYPE... _args),
			               const std::string& _desc) {
				AbstractFunction* tmp = createAbstractFunctionDirect(_name, _desc, _func);
			}
			/**
			 * @brief A extern client connect on specific user
			 * @param[in] _clientSessionID Source session Id on the client
			 * @param[in] _userName User name of the client to connect
			 * @todo Set a relur like ==> service not availlable / service close / service maintenance / service right reject
			 */
			virtual void clientConnect(size_t _clientSessionID, const std::string& _userName) = 0;
			virtual void clientDisconnect(size_t _clientSessionID) = 0;
	};
	template<class JUS_TYPE_SERVICE, class JUS_USER_ACCESS>
	class ServiceType : public jus::Service {
		private:
			JUS_USER_ACCESS& m_getUserInterface;
			// no need of shared_ptr or unique_ptr (if service die all is lost and is client die, the gateway notify us...)
			std::map<size_t, JUS_TYPE_SERVICE*> m_interface;
			std::vector<AbstractFunction*> m_listFunction;
		public:
			template<class JUS_RETURN_VALUE,
			         class JUS_CLASS_TYPE,
			         class... JUS_FUNC_ARGS_TYPE>
			void advertise(const std::string& _name,
			               JUS_RETURN_VALUE (JUS_CLASS_TYPE::*_func)(JUS_FUNC_ARGS_TYPE... _args),
			               const std::string& _desc) {
				// TODO: check if fucntion does not exist ...
				AbstractFunction* tmp = createAbstractFunctionClass(_name, _desc, _func);
				m_listFunction.push_back(tmp);
			}
			ServiceType(JUS_USER_ACCESS& _interface):
			  m_getUserInterface(_interface) {
				
			}
			void clientConnect(size_t _clientSessionID, const std::string& _userName) {
				// TODO : Set a mutex ...
				m_interface.insert(std::make_pair(_clientSessionID, new JUS_TYPE_SERVICE(m_getUserInterface.getUser(_userName))));
			}
			void clientDisconnect(size_t _clientSessionID) {
				auto it = m_interface.find(_clientSessionID);
				if (it != m_interface.end()) {
					// noting to do ==> user never conected.
					return;
				}
				// TODO : Set a mutex ...
				m_interface.erase(it);
			}
			ejson::Object callJson2(size_t _clientSessionID, const ejson::Object& _obj) {
				ejson::Object out;
				auto it = m_interface.find(_clientSessionID);
				if (it == m_interface.end()) {
					out.add("error", ejson::String("CLIENT-UNKNOW"));
					return out;
				}
				std::string call = _obj["call"].toString().get();
				const ejson::Array param = _obj["param"].toArray();
				for (auto &it2 : m_listFunction) {
					if (it2 == nullptr) {
						continue;
					}
					if (it2->getName() != call) {
						continue;
					}
					JUS_TYPE_SERVICE* elem = it->second;
					return it2->executeJson(param, (void*)elem).toObject();
				}
				out.add("error", ejson::String("FUNCTION-UNKNOW"));
				return out;
			}
	};
}



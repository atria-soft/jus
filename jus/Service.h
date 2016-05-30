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
#include <jus/RemoteProcessCall.h>

namespace jus {
	class ClientProperty {
		public:
			ClientProperty(const std::string& _clientName="", const std::vector<std::string>& _groups = std::vector<std::string>()) :
			  m_name(_clientName),
			  m_groups(_groups) {
				
			}
		private:
			std::string m_name;
		public:
			void setName(const std::string& _name) {
				m_name = _name;
			}
			const std::string& getName() {
				return m_name;
			}
		private:
			std::vector<std::string> m_groups;
		public:
			void setGroups(std::vector<std::string> _groups) {
				m_groups = _groups;
			}
			const std::vector<std::string>& getGroups() {
				return m_groups;
			}
		private:
			std::vector<std::string> m_listAthorizedFunction;
		public:
			void addAuthorized(const std::string& _funcName) {
				m_listAthorizedFunction.push_back(_funcName);
			}
			bool isFunctionAuthorized(const std::string& _funcName) {
				return std::find(m_listAthorizedFunction.begin(), m_listAthorizedFunction.end(), _funcName) != m_listAthorizedFunction.end();
			}
	};
}
namespace jus {
	class Service : public eproperty::Interface, public jus::RemoteProcessCall {
		protected:
			std::mutex m_mutex;
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
		private:
			jus::TcpString m_interfaceClient;
			uint32_t m_id;
			std::vector<std::string> m_newData;
		public:
			Service();
			virtual ~Service();
			void connect(const std::string& _serviceName, uint32_t _numberRetry = 1);
			void disconnect();
		private:
			void onClientData(std::string _value);
			std::string asyncRead();
		public:
			void pingIsAlive();
			bool GateWayAlive();
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
			/**
			 * @brief A extern client connect on specific user
			 * @param[in] _clientSessionID Source session Id on the client
			 * @param[in] _userName User name of the client to connect
			 * @todo Set a relur like ==> service not availlable / service close / service maintenance / service right reject
			 */
			virtual void clientConnect(size_t _clientSessionID, const std::string& _userName, const std::string& _clientName, const std::vector<std::string>& _groups) = 0;
			virtual void clientDisconnect(size_t _clientSessionID) = 0;
			// Genenric function call:
			ejson::Value callJson(const ejson::Object& _obj);
			virtual ejson::Object callJson2(size_t _clientId, const ejson::Object& _obj) = 0;
			std::vector<std::string> getExtention();
		public:
			// Add Local fuction (depend on this class)
			template<class JUS_RETURN_VALUE,
			         class JUS_CLASS_TYPE,
			         class... JUS_FUNC_ARGS_TYPE>
			void advertise(std::string _name,
			               JUS_RETURN_VALUE (JUS_CLASS_TYPE::*_func)(JUS_FUNC_ARGS_TYPE... _args),
			               const std::string& _desc = "") {
				_name = "srv." + _name;
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					if (it->getName() == _name) {
						JUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return;
					}
				}
				AbstractFunction* tmp = createAbstractFunctionClass(_name, _desc, _func);
				if (tmp == nullptr) {
					JUS_ERROR("can not create abstract function ... '" << _name << "'");
					return;
				}
				tmp->setType(jus::AbstractFunction::type::service);
				JUS_INFO("Add function '" << _name << "' in local mode");
				m_listFunction.push_back(tmp);
			}
	};
	template<class JUS_TYPE_SERVICE, class JUS_USER_ACCESS>
	class ServiceType : public jus::Service {
		private:
			JUS_USER_ACCESS& m_getUserInterface;
			// no need of shared_ptr or unique_ptr (if service die all is lost and is client die, the gateway notify us...)
			std::map<uint64_t, std::pair<ememory::SharedPtr<ClientProperty>, ememory::SharedPtr<JUS_TYPE_SERVICE>>> m_interface;
		public:
			template<class JUS_RETURN_VALUE,
			         class JUS_CLASS_TYPE,
			         class... JUS_FUNC_ARGS_TYPE>
			void advertise(const std::string& _name,
			               JUS_RETURN_VALUE (JUS_CLASS_TYPE::*_func)(JUS_FUNC_ARGS_TYPE... _args),
			               const std::string& _desc = "") {
				if (etk::start_with(_name, "srv.") == true) {
					JUS_ERROR("Advertise function start with 'srv.' is not permited ==> only allow for internal service: '" << _name << "'");
					return;
				}
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					if (it->getName() == _name) {
						JUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return;
					}
				}
				AbstractFunction* tmp = createAbstractFunctionClass(_name, _desc, _func);
				if (tmp == nullptr) {
					JUS_ERROR("can not create abstract function ... '" << _name << "'");
					return;
				}
				tmp->setType(jus::AbstractFunction::type::object);
				JUS_INFO("Add function '" << _name << "' in object mode");
				m_listFunction.push_back(tmp);
			}
			ServiceType(JUS_USER_ACCESS& _interface):
			  m_getUserInterface(_interface) {
				
			}
			bool isFunctionAuthorized(uint64_t _clientSessionID, const std::string& _funcName) {
				auto it = m_interface.find(_clientSessionID);
				if (it == m_interface.end()) {
					return false;
				}
				return it->second.first->isFunctionAuthorized(_funcName);
			}
			void clientConnect(uint64_t _clientSessionID, const std::string& _userName, const std::string& _clientName, const std::vector<std::string>& _groups) {
				std::unique_lock<std::mutex> lock(m_mutex);
				JUS_DEBUG("connect: " << _clientSessionID << " to '" << _userName << "'");
				JUS_DEBUG("    client name='" << _clientName << "'");
				JUS_DEBUG("    groups=" << etk::to_string(_groups));
				ememory::SharedPtr<ClientProperty> tmpProperty = std::make_shared<ClientProperty>(_clientName, _groups);
				ememory::SharedPtr<JUS_TYPE_SERVICE> tmpSrv = std::make_shared<JUS_TYPE_SERVICE>(m_getUserInterface.getUser(_userName), tmpProperty);
				m_interface.insert(std::make_pair(_clientSessionID, std::make_pair(tmpProperty, tmpSrv)));
				// enable list of function availlable: 
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					tmpProperty->addAuthorized(it->getName());
				}
			}
			void clientDisconnect(uint64_t _clientSessionID) {
				std::unique_lock<std::mutex> lock(m_mutex);
				JUS_DEBUG("disconnect: " << _clientSessionID);
				auto it = m_interface.find(_clientSessionID);
				if (it == m_interface.end()) {
					JUS_WARNING("disconnect ==> Not find Client ID " << _clientSessionID);
					// noting to do ==> user never conected.
					return;
				}
				m_interface.erase(it);
			}
			void clientSetName(uint64_t _clientSessionID, const std::string& _clientName) {
				std::unique_lock<std::mutex> lock(m_mutex);
				auto it = m_interface.find(_clientSessionID);
				if (it == m_interface.end()) {
					JUS_ERROR("Change the client property but client was not created ...");
					return;
				}
				it->second.first->setName(_clientName);
			}
			void clientSetGroup(uint64_t _clientSessionID, const std::vector<std::string>& _clientGroups) {
				std::unique_lock<std::mutex> lock(m_mutex);
				auto it = m_interface.find(_clientSessionID);
				if (it == m_interface.end()) {
					JUS_ERROR("Change the client property but client was not created ...");
					return;
				}
				it->second.first->setGroups(_clientGroups);
			}
			ejson::Object callJson2(uint64_t _clientSessionID, const ejson::Object& _obj) {
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
					switch (it2->getType()) {
						case jus::AbstractFunction::type::object: {
							JUS_TYPE_SERVICE* elem = it->second.second.get();
							return it2->executeJson(param, (void*)elem).toObject();
						}
						case jus::AbstractFunction::type::local: {
							return it2->executeJson(param, (void*)((RemoteProcessCall*)this)).toObject();
						}
						case jus::AbstractFunction::type::service: {
							return it2->executeJson(param, (void*)this).toObject();
						}
						case jus::AbstractFunction::type::global: {
							return it2->executeJson(param, nullptr).toObject();
						}
						case jus::AbstractFunction::type::unknow:
							JUS_ERROR("Can not call unknow type ...");
							break;
					}
				}
				out.add("error", ejson::String("FUNCTION-UNKNOW"));
				return out;
			}
	};
}



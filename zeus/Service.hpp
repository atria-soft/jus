/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <eproperty/Value.hpp>
#include <zeus/AbstractFunctionTypeDirect.hpp>
#include <zeus/AbstractFunctionTypeClass.hpp>
#include <zeus/debug.hpp>
#include <zeus/RemoteProcessCall.hpp>
#include <zeus/Future.hpp>

/**
 * @brief Main zeus library namespace
 */
namespace zeus {
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	class ClientProperty {
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			ClientProperty(const std::string& _clientName="", const std::vector<std::string>& _groups = std::vector<std::string>()) :
			  m_name(_clientName),
			  m_groups(_groups) {
				
			}
		private:
			std::string m_name;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void setName(const std::string& _name) {
				m_name = _name;
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			const std::string& getName() {
				return m_name;
			}
		private:
			std::vector<std::string> m_groups;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void setGroups(std::vector<std::string> _groups) {
				m_groups = _groups;
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			const std::vector<std::string>& getGroups() {
				return m_groups;
			}
		private:
			std::vector<std::string> m_listAthorizedFunction;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void addAuthorized(const std::string& _funcName) {
				m_listAthorizedFunction.push_back(_funcName);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool isFunctionAuthorized(const std::string& _funcName) {
				ZEUS_ERROR("plop: " << _funcName << "  " << m_listAthorizedFunction);
				return std::find(m_listAthorizedFunction.begin(), m_listAthorizedFunction.end(), _funcName) != m_listAthorizedFunction.end();
			}
	};
}

namespace zeus {
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	class Service : public eproperty::Interface, public zeus::RemoteProcessCall {
		protected:
			std::mutex m_mutex;
		public:
			eproperty::Value<std::string> propertyIp; //!< Ip of WebSocket TCP connection
			eproperty::Value<uint16_t> propertyPort; //!< Port of the WebSocket connection
			eproperty::Value<std::string> propertyNameService; //!< Service name
		protected:
			ememory::SharedPtr<zeus::WebServer> m_interfaceClient;
			uint32_t m_id;
			std::vector<std::string> m_newData;
			std::vector<zeus::FutureBase> m_callMultiData;
			std::string m_nameUser;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			Service();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			virtual ~Service();
			/**
			 * @brief 
			 * @param[in] 
			 * @return true The connection is done corectly, false otherwise
			 */
			bool connect(uint32_t _numberRetry = 1);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void disconnect();
		private:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void onClientData(ememory::SharedPtr<zeus::Buffer> _value);
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
			bool GateWayAlive();
		private:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void onPropertyChangeServiceName();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void onPropertyChangeIp();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void onPropertyChangePort();
			/**
			 * @brief A extern client connect on specific user
			 * @param[in] _clientId Source session Id on the client
			 * @param[in] _userName User name of the client to connect
			 * @todo Set a relur like ==> service not availlable / service close / service maintenance / service right reject
			 */
			virtual void clientConnect(uint64_t _clientId, const std::string& _userName, const std::string& _clientName, const std::vector<std::string>& _groups) = 0;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			virtual void clientDisconnect(uint64_t _clientId) = 0;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void callBinary(ememory::SharedPtr<zeus::Buffer> _obj);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			virtual void callBinary2(const std::string& _call, ememory::SharedPtr<zeus::BufferCall> _obj) = 0;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			std::vector<std::string> getExtention();
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			// Add Local fuction (depend on this class)
			template<class ZEUS_RETURN_VALUE,
			         class ZEUS_CLASS_TYPE,
			         class... ZEUS_FUNC_ARGS_TYPE>
			zeus::AbstractFunction* advertise(std::string _name,
			                                  ZEUS_RETURN_VALUE (ZEUS_CLASS_TYPE::*_func)(ZEUS_FUNC_ARGS_TYPE... _args)) {
				_name = "srv." + _name;
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					if (it->getName() == _name) {
						ZEUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return nullptr;
					}
				}
				AbstractFunction* tmp = createAbstractFunctionClass(_name, _func);
				if (tmp == nullptr) {
					ZEUS_ERROR("can not create abstract function ... '" << _name << "'");
					return nullptr;
				}
				tmp->setType(zeus::AbstractFunction::type::service);
				ZEUS_INFO("Add function '" << _name << "' in local mode");
				m_listFunction.push_back(tmp);
				return tmp;
			}
			
	};
	template<class ZEUS_TYPE_SERVICE>
	class ServiceType : public zeus::Service {
		private:
			std::function<ememory::SharedPtr<ZEUS_TYPE_SERVICE>(ememory::SharedPtr<ClientProperty>, const std::string&)> m_factory;
		public:
			ServiceType(std::function<ememory::SharedPtr<ZEUS_TYPE_SERVICE>(ememory::SharedPtr<ClientProperty>, const std::string&)> _factory) {
				m_factory = _factory;
			}
		private:
			// no need of shared_ptr or unique_ptr (if service die all is lost and is client die, the gateway notify us...)
			std::map<uint64_t, std::pair<ememory::SharedPtr<ClientProperty>, ememory::SharedPtr<ZEUS_TYPE_SERVICE>>> m_interface;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class ZEUS_RETURN_VALUE,
			         class ZEUS_CLASS_TYPE,
			         class... ZEUS_FUNC_ARGS_TYPE>
			zeus::AbstractFunction* advertise(const std::string& _name,
			                                  ZEUS_RETURN_VALUE (ZEUS_CLASS_TYPE::*_func)(ZEUS_FUNC_ARGS_TYPE... _args)) {
				if (etk::start_with(_name, "srv.") == true) {
					ZEUS_ERROR("Advertise function start with 'srv.' is not permited ==> only allow for internal service: '" << _name << "'");
					return nullptr;
				}
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					if (it->getName() == _name) {
						ZEUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return nullptr;
					}
				}
				zeus::AbstractFunction* tmp = createAbstractFunctionClass(_name, _func);
				if (tmp == nullptr) {
					ZEUS_ERROR("can not create abstract function ... '" << _name << "'");
					return nullptr;
				}
				tmp->setType(zeus::AbstractFunction::type::object);
				ZEUS_INFO("Add function '" << _name << "' in object mode");
				m_listFunction.push_back(tmp);
				return tmp;
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool isFunctionAuthorized(uint64_t _clientId, const std::string& _funcName) {
				auto it = m_interface.find(_clientId);
				if (it == m_interface.end()) {
					ZEUS_ERROR("CLIENT does not exist ... " << _clientId << "  " << _funcName);
					return false;
				}
				return it->second.first->isFunctionAuthorized(_funcName);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void clientConnect(uint64_t _clientId, const std::string& _userName, const std::string& _clientName, const std::vector<std::string>& _groups) {
				std::unique_lock<std::mutex> lock(m_mutex);
				ZEUS_DEBUG("connect: " << _clientId << " to '" << _userName << "'");
				ZEUS_DEBUG("    client name='" << _clientName << "'");
				ZEUS_DEBUG("    groups=" << etk::to_string(_groups));
				ememory::SharedPtr<ClientProperty> tmpProperty = ememory::makeShared<ClientProperty>(_clientName, _groups);
				ememory::SharedPtr<ZEUS_TYPE_SERVICE> tmpSrv;
				if (m_factory != nullptr) {
					tmpSrv = m_factory(tmpProperty, m_nameUser);
				} else {
					ZEUS_ERROR("Create service with no factory");
				}
				m_interface.insert(std::make_pair(_clientId, std::make_pair(tmpProperty, tmpSrv)));
				// enable list of function availlable:
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					tmpProperty->addAuthorized(it->getName());
				}
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void clientDisconnect(uint64_t _clientId) {
				std::unique_lock<std::mutex> lock(m_mutex);
				ZEUS_DEBUG("disconnect: " << _clientId);
				auto it = m_interface.find(_clientId);
				if (it == m_interface.end()) {
					ZEUS_WARNING("disconnect ==> Not find Client ID " << _clientId);
					// noting to do ==> user never conected.
					return;
				}
				m_interface.erase(it);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void clientSetName(uint64_t _clientId, const std::string& _clientName) {
				std::unique_lock<std::mutex> lock(m_mutex);
				auto it = m_interface.find(_clientId);
				if (it == m_interface.end()) {
					ZEUS_ERROR("Change the client property but client was not created ...");
					return;
				}
				it->second.first->setName(_clientName);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void clientSetGroup(uint64_t _clientId, const std::vector<std::string>& _clientGroups) {
				std::unique_lock<std::mutex> lock(m_mutex);
				auto it = m_interface.find(_clientId);
				if (it == m_interface.end()) {
					ZEUS_ERROR("Change the client property but client was not created ...");
					return;
				}
				it->second.first->setGroups(_clientGroups);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void callBinary2(const std::string& _call, ememory::SharedPtr<zeus::BufferCall> _obj) {
				auto it = m_interface.find(_obj->getClientId());
				if (it == m_interface.end()) {
					m_interfaceClient->answerError(_obj->getTransactionId(), _obj->getClientId(), _obj->getServiceId(), "CLIENT-UNKNOW", "");
					return;
				}
				for (auto &it2 : m_listFunction) {
					if (it2 == nullptr) {
						continue;
					}
					if (it2->getName() != _call) {
						continue;
					}
					switch (it2->getType()) {
						case zeus::AbstractFunction::type::object: {
							ZEUS_TYPE_SERVICE* elem = it->second.second.get();
							it2->execute(m_interfaceClient, _obj, (void*)elem);
							return;
						}
						case zeus::AbstractFunction::type::local: {
							it2->execute(m_interfaceClient, _obj, (void*)((RemoteProcessCall*)this));
							return;
						}
						case zeus::AbstractFunction::type::service: {
							it2->execute(m_interfaceClient, _obj, (void*)this);
							return;
						}
						case zeus::AbstractFunction::type::global: {
							it2->execute(m_interfaceClient, _obj, nullptr);
							return;
						}
						case zeus::AbstractFunction::type::unknow:
							ZEUS_ERROR("Can not call unknow type ...");
							break;
					}
				}
				m_interfaceClient->answerError(_obj->getTransactionId(), _obj->getClientId(), _obj->getServiceId(), "FUNCTION-UNKNOW", "");
				return;
			}
	};
}



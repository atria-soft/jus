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
#include <zeus/Object.hpp>

/**
 * @brief Main zeus library namespace
 */
namespace zeus {
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	class ClientPropertyddd {
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			ClientPropertyddd(const std::string& _clientName="", const std::vector<std::string>& _groups = std::vector<std::string>()) :
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
	template<class ZEUS_TYPE_SERVICE>
	class ServiceType : public zeus::Object {
		public:
			//using factory = std::function<ememory::SharedPtr<ZEUS_TYPE_SERVICE>(uint16_t)>;
		private:
			// no need of shared_ptr or unique_ptr (if service die all is lost and is client die, the gateway notify us...)
			ememory::SharedPtr<ClientPropertyddd> m_property;
			ememory::SharedPtr<ZEUS_TYPE_SERVICE> m_interface;
		public:
			/*
			ServiceType(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _objectId, uint16_t _clientId, factory _factory) :
			  Object(_iface, _objectId) {
				m_interface = _factory(_clientId);
			}*/
			ServiceType(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _objectId, const ememory::SharedPtr<ZEUS_TYPE_SERVICE>& _element) :
			  Object(_iface, _objectId),
			  m_interface(_element) {
				// nothing else to do ...
			}
			/*
			ServiceType(zeus::Client* _client, uint16_t _objectId, ememory::makeShared<ZEUS_TYPE_SERVICE> _instance) :
			  Object(_client, _objectId),
			  m_interface(_instance) {
				
			}*/
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
				/*
				auto it = m_interface.find(_clientId);
				if (it == m_interface.end()) {
					ZEUS_ERROR("CLIENT does not exist ... " << _clientId << "  " << _funcName);
					return false;
				}
				return it->second.first->isFunctionAuthorized(_funcName);
				*/
				return true;
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			/*
			void clientConnect(uint16_t _sourceId, const std::string& _userName, const std::string& _clientName, const std::vector<std::string>& _groups) {
				std::unique_lock<std::mutex> lock(m_mutex);
				ZEUS_DEBUG("connect: " << _sourceId << " to '" << _userName << "'");
				ZEUS_DEBUG("    client name='" << _clientName << "'");
				ZEUS_DEBUG("    groups=" << etk::to_string(_groups));
				ememory::SharedPtr<ClientPropertyddd> tmpProperty = ememory::makeShared<ClientPropertyddd>(_clientName, _groups);
				ememory::SharedPtr<ZEUS_TYPE_SERVICE> tmpSrv;
				if (m_factory != nullptr) {
					tmpSrv = m_factory(tmpProperty, m_nameUser);
				} else {
					ZEUS_ERROR("Create service with no factory");
				}
				m_interface.insert(std::make_pair(_sourceId, std::make_pair(tmpProperty, tmpSrv)));
				// enable list of function availlable:
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					tmpProperty->addAuthorized(it->getName());
				}
			}
			*/
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			/*
			void clientDisconnect(uint16_t _sourceId) {
				std::unique_lock<std::mutex> lock(m_mutex);
				ZEUS_DEBUG("disconnect: " << _sourceId);
				auto it = m_interface.find(_sourceId);
				if (it == m_interface.end()) {
					ZEUS_WARNING("disconnect ==> Not find Client ID " << _sourceId);
					// noting to do ==> user never conected.
					return;
				}
				m_interface.erase(it);
			}
			*/
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			/*
			void clientSetName(uint16_t _sourceId, const std::string& _clientName) {
				std::unique_lock<std::mutex> lock(m_mutex);
				auto it = m_interface.find(_sourceId);
				if (it == m_interface.end()) {
					ZEUS_ERROR("Change the client property but client was not created ...");
					return;
				}
				it->second.first->setName(_clientName);
			}
			*/
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			/*
			void clientSetGroup(uint16_t _sourceId, const std::vector<std::string>& _clientGroups) {
				std::unique_lock<std::mutex> lock(m_mutex);
				auto it = m_interface.find(_sourceId);
				if (it == m_interface.end()) {
					ZEUS_ERROR("Change the client property but client was not created ...");
					return;
				}
				it->second.first->setGroups(_clientGroups);
			}
			*/
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void callBinary2(const std::string& _call, ememory::SharedPtr<zeus::BufferCall> _obj) {
				/*
				auto it = m_interface.find(_obj->getSourceId());
				if (it == m_interface.end()) {
					m_interfaceWeb->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "CLIENT-UNKNOW", "");
					return;
				}
				*/
				for (auto &it2 : m_listFunction) {
					if (it2 == nullptr) {
						continue;
					}
					if (it2->getName() != _call) {
						continue;
					}
					switch (it2->getType()) {
						case zeus::AbstractFunction::type::object: {
							ZEUS_TYPE_SERVICE* elem = m_interface.get();
							it2->execute(m_interfaceWeb, _obj, (void*)elem);
							return;
						}
						case zeus::AbstractFunction::type::local: {
							it2->execute(m_interfaceWeb, _obj, (void*)((RemoteProcessCall*)this));
							return;
						}
						case zeus::AbstractFunction::type::service: {
							it2->execute(m_interfaceWeb, _obj, (void*)this);
							return;
						}
						case zeus::AbstractFunction::type::global: {
							it2->execute(m_interfaceWeb, _obj, nullptr);
							return;
						}
						case zeus::AbstractFunction::type::unknow:
							ZEUS_ERROR("Can not call unknow type ...");
							break;
					}
				}
				m_interfaceWeb->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "FUNCTION-UNKNOW", "");
				return;
			}
	};
}



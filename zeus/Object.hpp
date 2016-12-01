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
#include <zeus/Client.hpp>

/**
 * @brief Main zeus library namespace
 */
namespace zeus {
	class Client;
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	class Object : public zeus::RemoteProcessCall {
		protected:
			std::mutex m_mutex;
		protected:
			uint16_t m_clientId; // TODO : Remove it
			uint16_t m_objectId; // TODO : Remove it
			std::vector<zeus::FutureBase> m_callMultiData;
		public:
			uint16_t getObjectId() { return m_objectId; }
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			//Object(zeus::Client* _client, uint16_t _objectId);
			Object(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _objectId);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			virtual ~Object();
		public:
			void receive(ememory::SharedPtr<zeus::Buffer> _value);
		private:
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
	
	template<class ZEUS_TYPE_OBJECT>
	class ObjectType : public zeus::Object {
		private:
			ememory::SharedPtr<ZEUS_TYPE_OBJECT> m_interface; // direct handle on the data;
		public:
			/*
			ObjectType(zeus::Client* _client, uint16_t _objectId, uint16_t _clientId) :
			  Object(_client, _objectId) {
				m_interface = ememory::makeShared<ZEUS_TYPE_OBJECT>(/ *_clientId* /);
			}
			*/
			ObjectType(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _objectId, const ememory::SharedPtr<ZEUS_TYPE_OBJECT>& _element) :
			  Object(_iface, _objectId),
			  m_interface(_element) {
				// nothing else to do ...
			}
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
			void callBinary2(const std::string& _call, ememory::SharedPtr<zeus::BufferCall> _obj) {
				for (auto &it2 : m_listFunction) {
					if (it2 == nullptr) {
						continue;
					}
					if (it2->getName() != _call) {
						continue;
					}
					switch (it2->getType()) {
						case zeus::AbstractFunction::type::object: {
							ZEUS_TYPE_OBJECT* elem = m_interface.get();
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
				m_interfaceWeb->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "FUNCTION-UNKNOW", "not find function name: '" + _call + "'");
				return;
			}
	};
}



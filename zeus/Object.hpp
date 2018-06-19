/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
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
	 * @brief An object is an element callable from the remote.
	 */
	class Object : public zeus::RemoteProcessCall {
		public:
			/**
			 * @brief Get the current object local unique ID
			 * @return The unique Object id
			 */
			uint16_t getObjectId() {
				return m_objectId;
			}
			/**
			 * @brief Contruct a new callable object
			 * @param[in] _iface Network interface
			 * @param[in] _objectId Unique Id of the object
			 */
			Object(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _objectId);
			/**
			 * @brief generic destructor
			 */
			virtual ~Object() = default;
		public:
			/**
			 * @brief Receive message to parse and execute
			 * @param[in] _value Message to process
			 */
			void receive(ememory::SharedPtr<zeus::Message> _value);
		private:
			/**
			 * @brief Specific call depending of the type of the object.
			 * @param[in] _call Name of the function that is called.
			 * @param[in] _value Message to process.
			 */
			virtual void callBinary(const etk::String& _call, ememory::SharedPtr<zeus::message::Call> _value) = 0;
		public:
			/**
			 * @brief Advertise a new function in the service/object ==> it is force the start with "obj.".
			 * @param[in] _name Name of the function
			 * @param[in] _func pointer on the function that might be used to call it.
			 * @return an handle on an abstract function that can be called.
			 */
			// Add Local fuction (depend on this class)
			template<class ZEUS_RETURN_VALUE,
			         class ZEUS_CLASS_TYPE,
			         class... ZEUS_FUNC_ARGS_TYPE>
			zeus::AbstractFunction* advertise(etk::String _name,
			                                  ZEUS_RETURN_VALUE (ZEUS_CLASS_TYPE::*_func)(ZEUS_FUNC_ARGS_TYPE... _args)) {
				_name = "obj." + _name;
				for (auto &it : m_listFunction) {
					if (it == null) {
						continue;
					}
					if (it->getName() == _name) {
						ZEUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return null;
					}
				}
				AbstractFunction* tmp = createAbstractFunctionClass(_name, _func);
				if (tmp == null) {
					ZEUS_ERROR("can not create abstract function ... '" << _name << "'");
					return null;
				}
				tmp->setType(zeus::AbstractFunction::type::service);
				ZEUS_VERBOSE("Add function '" << _name << "' in local mode");
				m_listFunction.pushBack(tmp);
				return tmp;
			}
	};
	/**
	 * @brief The object is all time different, and we need to called it corectly and keep it alive while the remote user need it.
	 *        The this class permit to have a a pointer on the temporary object.
	 */
	template<class ZEUS_TYPE_OBJECT>
	class ObjectType : public zeus::Object {
		private:
			ememory::SharedPtr<ZEUS_TYPE_OBJECT> m_interface; //!< handle on the object that might be called.
		public:
			ObjectType(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _objectId, const ememory::SharedPtr<ZEUS_TYPE_OBJECT>& _element) :
			  Object(_iface, _objectId),
			  m_interface(_element) {
				// nothing else to do ...
			}
		public:
			/**
			 * @brief Advertise a new signal that use a specific call processing order
			 * @param[in] _name Name of the function
			 * @param[in] _func Pointer on the function to call when name call is requested
			 * @return Pointer on the function abstraction call that is done
			 * @note: this is for ACTION function call not normal function call
			 */
			template<class ZEUS_RETURN_VALUE,
			         class ZEUS_ACTION_TYPE,
			         class ZEUS_CLASS_TYPE,
			         class... ZEUS_FUNC_ARGS_TYPE>
			zeus::AbstractFunction* advertise(const etk::String& _name,
			                                  ZEUS_RETURN_VALUE (ZEUS_CLASS_TYPE::*_func)(zeus::ActionNotification<ZEUS_ACTION_TYPE>& _notifs, ZEUS_FUNC_ARGS_TYPE... _args)) {
				if (etk::start_with(_name, "srv.") == true) {
					ZEUS_ERROR("Advertise function start with 'srv.' is not permited ==> only allow for internal service: '" << _name << "'");
					return null;
				}
				for (auto &it : m_listFunction) {
					if (it == null) {
						continue;
					}
					if (it->getName() == _name) {
						ZEUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return null;
					}
				}
				zeus::AbstractFunction* tmp = createAbstractFunctionClass(_name, _func);
				if (tmp == null) {
					ZEUS_ERROR("can not create abstract function ... '" << _name << "'");
					return null;
				}
				tmp->setType(zeus::AbstractFunction::type::object);
				ZEUS_VERBOSE("Add function '" << _name << "' in object mode");
				m_listFunction.pushBack(tmp);
				return tmp;
			}
			/**
			 * @brief Advertise a new signal that use a specific call processing order
			 * @param[in] _name Name of the function
			 * @param[in] _func Pointer on the function to call when name call is requested
			 * @return Pointer on the function abstraction call that is done
			 * @note: this is for normal function call not action call
			 */
			template<class ZEUS_RETURN_VALUE,
			         class ZEUS_CLASS_TYPE,
			         class... ZEUS_FUNC_ARGS_TYPE>
			zeus::AbstractFunction* advertise(const etk::String& _name,
			                                  ZEUS_RETURN_VALUE (ZEUS_CLASS_TYPE::*_func)(ZEUS_FUNC_ARGS_TYPE... _args)) {
				if (etk::start_with(_name, "srv.") == true) {
					ZEUS_ERROR("Advertise function start with 'srv.' is not permited ==> only allow for internal service: '" << _name << "'");
					return null;
				}
				for (auto &it : m_listFunction) {
					if (it == null) {
						continue;
					}
					if (it->getName() == _name) {
						ZEUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return null;
					}
				}
				zeus::AbstractFunction* tmp = createAbstractFunctionClass(_name, _func);
				if (tmp == null) {
					ZEUS_ERROR("can not create abstract function ... '" << _name << "'");
					return null;
				}
				tmp->setType(zeus::AbstractFunction::type::object);
				ZEUS_VERBOSE("Add function '" << _name << "' in object mode");
				m_listFunction.pushBack(tmp);
				return tmp;
			}
			bool isFunctionAuthorized(uint64_t _clientId, const etk::String& _funcName) override {
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
			void callBinary(const etk::String& _call, ememory::SharedPtr<zeus::message::Call> _value) override {
				for (auto &it2 : m_listFunction) {
					if (it2 == null) {
						continue;
					}
					if (it2->getName() != _call) {
						continue;
					}
					// TODO: Check if client is athorized ...
					// depending on where the function is defined, the call is not the same ...
					switch (it2->getType()) {
						case zeus::AbstractFunction::type::object: {
							ZEUS_TYPE_OBJECT* elem = m_interface.get();
							it2->execute(m_interfaceWeb, _value, (void*)elem);
							return;
						}
						case zeus::AbstractFunction::type::local: {
							it2->execute(m_interfaceWeb, _value, (void*)((RemoteProcessCall*)this));
							return;
						}
						case zeus::AbstractFunction::type::service: {
							it2->execute(m_interfaceWeb, _value, (void*)this);
							return;
						}
						case zeus::AbstractFunction::type::global: {
							it2->execute(m_interfaceWeb, _value, null);
							return;
						}
						case zeus::AbstractFunction::type::unknow:
							ZEUS_ERROR("Can not call unknow type ...");
							break;
					}
				}
				m_interfaceWeb->answerError(_value->getTransactionId(),
				                            _value->getDestination(),
				                            _value->getSource(),
				                            "FUNCTION-UNKNOW",
				                            "not find function name: '" + _call + "'");
			}
	};
}



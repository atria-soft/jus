/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <zeus/debug.hpp>
#include <zeus/AbstractFunction.hpp>
#include <zeus/mineType.hpp>
#include <zeus/ActionNotification.hpp>
#include <etk/Exception.hpp>


namespace zeus {
	/**
	 * @brief Execute a call on the function with a return value
	 * @param[in] _interfaceClient Web interface to send data
	 * @param[in] _obj Message input call (that have parameter already check)
	 * @param[in] _pointer Pointer on the class to call
	 * @param[in] _func pointer on the function to call
	 */
	template <class ZEUS_CLASS_TYPE, class ZEUS_RETURN, class ZEUS_EVENT, class... ZEUS_TYPES>
	void executeClassCall(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
	                      ememory::SharedPtr<zeus::message::Parameter> _obj,
	                      ZEUS_CLASS_TYPE* _pointer,
	                      ZEUS_RETURN (ZEUS_CLASS_TYPE::*_func)(zeus::ActionNotification<ZEUS_EVENT>& _notifs, ZEUS_TYPES...)) {
		if (_obj == null) {
			return;
		}
		ZEUS_RETURN ret;
		zeus::ActionNotification<ZEUS_EVENT> notifs(_interfaceClient, _obj->getTransactionId(), _obj->getDestination(), _obj->getSource());
		if (zeus::checkOrderFunctionParameter() == true) {
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			ret = (*_pointer.*_func)(notifs, _obj->getParameter<ZEUS_TYPES>(idParam++)...);
		} else {
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			ret = (*_pointer.*_func)(notifs, _obj->getParameter<ZEUS_TYPES>(idParam--)...);
		}
		if (_interfaceClient == null) {
			ZEUS_ERROR("Nullptr for _interfaceClient");
			return;
		}
		_interfaceClient->addAsync([=](WebServer* _interface) {
			    _interface->answerValue(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), ret);
			    return true;
			});
	}
	/**
	 * @brief Execute a call on the function with a return value
	 * @param[in] _interfaceClient Web interface to send data
	 * @param[in] _obj Message input call (that have parameter already check)
	 * @param[in] _pointer Pointer on the class to call
	 * @param[in] _func pointer on the function to call
	 */
	template <class ZEUS_CLASS_TYPE, class ZEUS_RETURN, class... ZEUS_TYPES>
	void executeClassCall(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
	                      ememory::SharedPtr<zeus::message::Parameter> _obj,
	                      ZEUS_CLASS_TYPE* _pointer,
	                      ZEUS_RETURN (ZEUS_CLASS_TYPE::*_func)(ZEUS_TYPES...)) {
		if (_obj == null) {
			return;
		}
		ZEUS_RETURN ret;
		if (zeus::checkOrderFunctionParameter() == true) {
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			ret = (*_pointer.*_func)(_obj->getParameter<ZEUS_TYPES>(idParam++)...);
		} else {
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			ret = (*_pointer.*_func)(_obj->getParameter<ZEUS_TYPES>(idParam--)...);
		}
		if (_interfaceClient == null) {
			ZEUS_ERROR("Nullptr for _interfaceClient");
			return;
		}
		_interfaceClient->addAsync([=](WebServer* _interface) {
			    _interface->answerValue(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), ret);
			    return true;
			});
	}
	/**
	 * @brief Execute a call on the function with NO return value
	 * @param[in] _interfaceClient Web interface to send data
	 * @param[in] _obj Message input call (that have parameter already check)
	 * @param[in] _pointer Pointer on the class to call
	 * @param[in] _func pointer on the function to call
	 */
	template <class ZEUS_CLASS_TYPE, class ZEUS_EVENT, class... ZEUS_TYPES>
	void executeClassCall(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
	                      ememory::SharedPtr<zeus::message::Parameter> _obj,
	                      ZEUS_CLASS_TYPE* _pointer,
	                      void (ZEUS_CLASS_TYPE::*_func)(zeus::ActionNotification<ZEUS_EVENT>& _notifs, ZEUS_TYPES...)) {
		if (_obj == null) {
			return;
		}
		zeus::ActionNotification<ZEUS_EVENT> notifs(_interfaceClient, _obj->getTransactionId(), _obj->getDestination(), _obj->getSource());
		if (zeus::checkOrderFunctionParameter() == true) {
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			(*_pointer.*_func)(notifs, _obj->getParameter<ZEUS_TYPES>(idParam++)...);
		} else {
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			(*_pointer.*_func)(notifs, _obj->getParameter<ZEUS_TYPES>(idParam--)...);
		}
		if (_interfaceClient == null) {
			ZEUS_ERROR("Nullptr for _interfaceClient");
			return;
		}
		_interfaceClient->addAsync([=](WebServer* _interface) {
			    _interface->answerVoid(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource());
			    return true;
			});
	}
	/**
	 * @brief Execute a call on the function with NO return value
	 * @param[in] _interfaceClient Web interface to send data
	 * @param[in] _obj Message input call (that have parameter already check)
	 * @param[in] _pointer Pointer on the class to call
	 * @param[in] _func pointer on the function to call
	 */
	template <class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	void executeClassCall(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
	                      ememory::SharedPtr<zeus::message::Parameter> _obj,
	                      ZEUS_CLASS_TYPE* _pointer,
	                      void (ZEUS_CLASS_TYPE::*_func)(ZEUS_TYPES...)) {
		if (_obj == null) {
			return;
		}
		if (zeus::checkOrderFunctionParameter() == true) {
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			(*_pointer.*_func)(_obj->getParameter<ZEUS_TYPES>(idParam++)...);
		} else {
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			(*_pointer.*_func)(_obj->getParameter<ZEUS_TYPES>(idParam--)...);
		}
		if (_interfaceClient == null) {
			ZEUS_ERROR("Nullptr for _interfaceClient");
			return;
		}
		_interfaceClient->addAsync([=](WebServer* _interface) {
			    _interface->answerVoid(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource());
			    return true;
			});
	}
	/**
	 * @brief Chass that permit to declare a function that call intanced element or a class element
	 */
	template <class ZEUS_RETURN, class ZEUS_EVENT, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	class AbstractActionTypeClass: public zeus::AbstractFunction {
		protected:
			static const zeus::message::ParamType m_returnType;
			static const zeus::message::ParamType m_paramType[sizeof...(ZEUS_TYPES)];
		public:
			using functionType = ZEUS_RETURN (ZEUS_CLASS_TYPE::*)(zeus::ActionNotification<ZEUS_EVENT>& _notifs, ZEUS_TYPES...);
			functionType m_function;
			/**
			 * @brief Constructor
			 * @param[in] _name Name of the function
			 * @param[in] _fptr Pointer on the function
			 */
			AbstractActionTypeClass(const etk::String& _name, functionType _fptr):
			  AbstractFunction(_name),
			  m_function(_fptr) {
			}
			etk::String getPrototypeReturn() const override {
				return m_returnType.getName();
			}
			etk::Vector<etk::String> getPrototypeParam() const override {
				etk::Vector<etk::String> out;
				for (size_t iii=0; iii<sizeof...(ZEUS_TYPES); ++iii) {
					out.pushBack(m_paramType[iii].getName());
				}
				return out;
			}
			void execute(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
			             ememory::SharedPtr<zeus::message::Call> _obj,
			             void* _class) override {
				if (_obj == null) {
					return;
				}
				ZEUS_CLASS_TYPE* tmpClass = null;
				if (_class != null) {
					tmpClass = (ZEUS_CLASS_TYPE*)_class;
				}
				
				if (_interfaceClient == null) {
					ZEUS_ERROR("Nullptr for _interfaceWeb");
					return;
				}
				// check parameter number
				if (_obj->getNumberParameter() != sizeof...(ZEUS_TYPES)) {
					ZEUS_ERROR("Wrong number of Parameters ...");
					etk::String help = "request ";
					help += etk::toString(_obj->getNumberParameter());
					help += " parameters and need ";
					help += etk::toString(sizeof...(ZEUS_TYPES));
					help += " parameters. prototype function:";
					help += getPrototype();
					_interfaceClient->answerError(_obj->getTransactionId(),
					                              _obj->getDestination(),
					                              _obj->getSource(),
					                              "WRONG-PARAMETER-NUMBER",
					                              help);
					return;
				}
				// check parameter compatibility
				for (size_t iii=0; iii<sizeof...(ZEUS_TYPES); ++iii) {
					if (checkCompatibility(m_paramType[iii], _obj->getParameterType(iii)) == false) {
						_interfaceClient->answerError(_obj->getTransactionId(),
						                              _obj->getDestination(),
						                              _obj->getSource(),
						                              "WRONG-PARAMETER-TYPE",
						                              etk::String("Parameter id ") + etk::toString(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'");
						return;
					}
				}
				ethread::metadataSet(zeus::g_threadKeyTransactionId, _obj->getTransactionId());
				ethread::metadataSet(zeus::g_threadKeyTransactionSource, _obj->getSource());
				ethread::metadataSet(zeus::g_threadKeyTransactionDestination, _obj->getDestination());
				try {
					// execute cmd:
					zeus::executeClassCall(_interfaceClient, _obj, tmpClass, m_function);
				} catch (const etk::Exception& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), eee.which(), eee.what());
						    return true;
						});
				}
				ethread::metadataRemove(zeus::g_threadKeyTransactionId);
				ethread::metadataRemove(zeus::g_threadKeyTransactionSource);
				ethread::metadataRemove(zeus::g_threadKeyTransactionDestination);
			}
	};
	/**
	 * @brief Chass that permit to declare a function that call intanced element or a class element
	 */
	template <class ZEUS_RETURN, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	class AbstractFunctionTypeClass: public zeus::AbstractFunction {
		protected:
			static const zeus::message::ParamType m_returnType;
			static const zeus::message::ParamType m_paramType[sizeof...(ZEUS_TYPES)];
		public:
			using functionType = ZEUS_RETURN (ZEUS_CLASS_TYPE::*)(ZEUS_TYPES...);
			functionType m_function;
			/**
			 * @brief Constructor
			 * @param[in] _name Name of the function
			 * @param[in] _fptr Pointer on the function
			 */
			AbstractFunctionTypeClass(const etk::String& _name, functionType _fptr):
			  AbstractFunction(_name),
			  m_function(_fptr) {
			}
			etk::String getPrototypeReturn() const override {
				return m_returnType.getName();
			}
			etk::Vector<etk::String> getPrototypeParam() const override {
				etk::Vector<etk::String> out;
				for (size_t iii=0; iii<sizeof...(ZEUS_TYPES); ++iii) {
					out.pushBack(m_paramType[iii].getName());
				}
				return out;
			}
			void execute(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
			             ememory::SharedPtr<zeus::message::Call> _obj,
			             void* _class) override {
				if (_obj == null) {
					return;
				}
				ZEUS_CLASS_TYPE* tmpClass = null;
				if (_class != null) {
					tmpClass = (ZEUS_CLASS_TYPE*)_class;
				}
				
				if (_interfaceClient == null) {
					ZEUS_ERROR("Nullptr for _interfaceWeb");
					return;
				}
				// check parameter number
				if (_obj->getNumberParameter() != sizeof...(ZEUS_TYPES)) {
					ZEUS_ERROR("Wrong number of Parameters ...");
					etk::String help = "request ";
					help += etk::toString(_obj->getNumberParameter());
					help += " parameters and need ";
					help += etk::toString(sizeof...(ZEUS_TYPES));
					help += " parameters. prototype function:";
					help += getPrototype();
					_interfaceClient->answerError(_obj->getTransactionId(),
					                              _obj->getDestination(),
					                              _obj->getSource(),
					                              "WRONG-PARAMETER-NUMBER",
					                              help);
					return;
				}
				// check parameter compatibility
				for (size_t iii=0; iii<sizeof...(ZEUS_TYPES); ++iii) {
					if (checkCompatibility(m_paramType[iii], _obj->getParameterType(iii)) == false) {
						_interfaceClient->answerError(_obj->getTransactionId(),
						                              _obj->getDestination(),
						                              _obj->getSource(),
						                              "WRONG-PARAMETER-TYPE",
						                              etk::String("Parameter id ") + etk::toString(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'");
						return;
					}
				}
				ethread::metadataSet(zeus::g_threadKeyTransactionId, _obj->getTransactionId());
				ethread::metadataSet(zeus::g_threadKeyTransactionSource, _obj->getSource());
				ethread::metadataSet(zeus::g_threadKeyTransactionDestination, _obj->getDestination());
				try {
					// execute cmd:
					zeus::executeClassCall(_interfaceClient, _obj, tmpClass, m_function);
				} catch (const etk::Exception& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), eee.which(), eee.what());
						    return true;
						});
				}
				ethread::metadataRemove(zeus::g_threadKeyTransactionId);
				ethread::metadataRemove(zeus::g_threadKeyTransactionSource);
				ethread::metadataRemove(zeus::g_threadKeyTransactionDestination);
			}
	};
	// specialization
	template <class ZEUS_RETURN, class ZEUS_EVENT, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	const zeus::message::ParamType AbstractActionTypeClass<ZEUS_RETURN, ZEUS_EVENT, ZEUS_CLASS_TYPE, ZEUS_TYPES...>::m_returnType = zeus::message::createType<ZEUS_RETURN>();
	// specialization
	template <class ZEUS_RETURN, class ZEUS_EVENT, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	const zeus::message::ParamType AbstractActionTypeClass<ZEUS_RETURN, ZEUS_EVENT, ZEUS_CLASS_TYPE, ZEUS_TYPES...>::m_paramType[sizeof...(ZEUS_TYPES)] = {zeus::message::createType<ZEUS_TYPES>()...};
	// specialization
	template <class ZEUS_RETURN, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	const zeus::message::ParamType AbstractFunctionTypeClass<ZEUS_RETURN, ZEUS_CLASS_TYPE, ZEUS_TYPES...>::m_returnType = zeus::message::createType<ZEUS_RETURN>();
	// specialization
	template <class ZEUS_RETURN, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	const zeus::message::ParamType AbstractFunctionTypeClass<ZEUS_RETURN, ZEUS_CLASS_TYPE, ZEUS_TYPES...>::m_paramType[sizeof...(ZEUS_TYPES)] = {zeus::message::createType<ZEUS_TYPES>()...};
	/**
	 * @brief Create a function information with the function type
	 * @param[in] _name Name of the function
	 * @param[in] _fffp Pointer of the function
	 * @return Abstract type of the function
	 */
	template <typename ZEUS_RETURN, class ZEUS_EVENT, class ZEUS_CLASS_TYPE, typename... ZEUS_TYPES>
	AbstractFunction* createAbstractFunctionClass(const etk::String& _name, ZEUS_RETURN (ZEUS_CLASS_TYPE::*_fffp)(zeus::ActionNotification<ZEUS_EVENT>& _notifs, ZEUS_TYPES...)) {
		return new AbstractActionTypeClass<ZEUS_RETURN, ZEUS_EVENT, ZEUS_CLASS_TYPE, ZEUS_TYPES...>(_name, _fffp);
	}
	/**
	 * @brief Create a function information with the function type
	 * @param[in] _name Name of the function
	 * @param[in] _fffp Pointer of the function
	 * @return Abstract type of the function
	 */
	template <typename ZEUS_RETURN, class ZEUS_CLASS_TYPE, typename... ZEUS_TYPES>
	AbstractFunction* createAbstractFunctionClass(const etk::String& _name, ZEUS_RETURN (ZEUS_CLASS_TYPE::*_fffp)(ZEUS_TYPES...)) {
		return new AbstractFunctionTypeClass<ZEUS_RETURN, ZEUS_CLASS_TYPE, ZEUS_TYPES...>(_name, _fffp);
	}
}


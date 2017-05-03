/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <zeus/debug.hpp>
#include <zeus/AbstractFunction.hpp>

namespace zeus {
	/**
	 * @brief Execute a call on the global function with a return value
	 * @param[in] _interfaceClient Web interface to send data
	 * @param[in] _obj Message input call (that have parameter already check)
	 * @param[in] _func pointer on the function to call
	 */
	template <class ZEUS_RETURN, class... ZEUS_TYPES>
	void executeCall(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
	                 ememory::SharedPtr<zeus::message::Parameter> _obj,
	                 ZEUS_RETURN (*_func)(ZEUS_TYPES...)) {
		if (_obj == nullptr) {
			return;
		}
		ZEUS_RETURN ret;
		if (zeus::checkOrderFunctionParameter() == true) {
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			ret = _func(_obj->getParameter<ZEUS_TYPES>(idParam++)...);
		} else {
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			ret = _func(_obj->getParameter<ZEUS_TYPES>(idParam--)...);
		}
		if (_interfaceClient == nullptr) {
			ZEUS_ERROR("Nullptr for _interfaceWeb");
			return;
		}
		_interfaceClient->addAsync([=](WebServer* _interface) {
			    _interface->answerValue(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), ret);
			    return true;
			});
	}
	/**
	 * @brief Execute a call on the global function with NO return value
	 * @param[in] _interfaceClient Web interface to send data
	 * @param[in] _obj Message input call (that have parameter already check)
	 * @param[in] _func pointer on the function to call
	 */
	template <class... ZEUS_TYPES>
	void executeCall(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
	                 ememory::SharedPtr<zeus::message::Parameter> _obj,
	                 void (*_func)(ZEUS_TYPES...)) {
		if (_obj == nullptr) {
			return;
		}
		if (zeus::checkOrderFunctionParameter() == true) {
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			_func(_obj->getParameter<ZEUS_TYPES>(idParam++)...);
		} else {
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			_func(_obj->getParameter<ZEUS_TYPES>(idParam--)...);
		}
		if (_interfaceClient == nullptr) {
			ZEUS_ERROR("Nullptr for _interfaceWeb");
			return;
		}
		_interfaceClient->addAsync([=](WebServer* _interface) {
			    _interface->answerVoid(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource());
			    return true;
			});
	}
	
	/**
	 * @brief Chass that permit to declare a function that call global function
	 */
	template <class ZEUS_RETURN, class... ZEUS_TYPES>
	class AbstractFunctionTypeDirect: public zeus::AbstractFunction {
		protected:
			static const zeus::message::ParamType m_returnType;
			static const zeus::message::ParamType m_paramType[sizeof...(ZEUS_TYPES)];
		public:
			using functionType = ZEUS_RETURN (*)(ZEUS_TYPES...);
			functionType m_function;
			/**
			 * @brief Constructor
			 * @param[in] _name Name of the function
			 * @param[in] _fptr Pointer on the function
			 */
			AbstractFunctionTypeDirect(const std::string& _name, functionType _fptr):
			  AbstractFunction(_name),
			  m_function(_fptr) {
			}
			std::string getPrototypeReturn() const override {
				return m_returnType.getName();
			}
			std::vector<std::string> getPrototypeParam() const override {
				std::vector<std::string> out;
				for (size_t iii=0; iii<sizeof...(ZEUS_TYPES); ++iii) {
					out.push_back(m_paramType[iii].getName());
				}
				return out;
			}
			void execute(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
			             ememory::SharedPtr<zeus::message::Call> _obj,
			             void* _class) override {
				if (_obj == nullptr) {
					return;
				}
				if (_interfaceClient == nullptr) {
					ZEUS_ERROR("Nullptr for _interfaceWeb");
					return;
				}
				// check parameter number
				if (_obj->getNumberParameter() != sizeof...(ZEUS_TYPES)) {
					std::string help = "request ";
					help += etk::to_string(_obj->getNumberParameter());
					help += " parameters and need ";
					help += etk::to_string(sizeof...(ZEUS_TYPES));
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
						                              std::string("Parameter id ") + etk::to_string(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'");
						return;
					}
				}
				ethread::metadataSet(zeus::g_threadKeyTransactionId, _obj->getTransactionId());
				ethread::metadataSet(zeus::g_threadKeyTransactionSource, _obj->getSource());
				ethread::metadataSet(zeus::g_threadKeyTransactionDestination, _obj->getDestination());
				try {
					// execute cmd:
					zeus::executeCall(_interfaceClient, _obj, m_function);
				} catch (const std::invalid_argument& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "INVALID-ARGUMENT", eee.what());
						    return true;
						});
				} catch (const std::domain_error& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "DOMAIN-ERROR", eee.what());
						    return true;
						});
				} catch (const std::length_error& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "LENGTH-ERROR", eee.what());
						    return true;
						});
				} catch (const std::out_of_range& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "OUT-OF-RANGE", eee.what());
						    return true;
						});
				} catch (const std::range_error& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "RANGE-ERROR", eee.what());
						    return true;
						});
				} catch (const std::overflow_error& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "OVERFLOW-ERROR", eee.what());
						    return true;
						});
				} catch (const std::underflow_error& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "UNDERFLOW-ERROR", eee.what());
						    return true;
						});
				} catch (const std::logic_error& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "LOGIC-ERROR", eee.what());
						    return true;
						});
				} catch (const std::runtime_error& eee) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "RUNTIME-ERROR", eee.what());
						    return true;
						});
				} catch ( ... ) {
					_interfaceClient->addAsync([=](WebServer* _interface) {
						    _interface->answerError(_obj->getTransactionId(), _obj->getDestination(), _obj->getSource(), "?-ERROR", "catch unknow error");
						    return true;
						});
				}
				ethread::metadataRemove(zeus::g_threadKeyTransactionId);
				ethread::metadataRemove(zeus::g_threadKeyTransactionSource);
				ethread::metadataRemove(zeus::g_threadKeyTransactionDestination);
			}
	};
	// specialization
	template <class ZEUS_RETURN, class... ZEUS_TYPES>
	const zeus::message::ParamType AbstractFunctionTypeDirect<ZEUS_RETURN, ZEUS_TYPES...>::m_returnType = zeus::message::createType<ZEUS_RETURN>();
	// specialization
	template <class ZEUS_RETURN, class... ZEUS_TYPES>
	const zeus::message::ParamType AbstractFunctionTypeDirect<ZEUS_RETURN, ZEUS_TYPES...>::m_paramType[sizeof...(ZEUS_TYPES)] = {zeus::message::createType<ZEUS_TYPES>()...};
	/**
	 * @brief Create a function information with the function type
	 * @param[in] _name Name of the function
	 * @param[in] _fffp Pointer of the function
	 * @return Abstract type of the function
	 */
	template <typename ZEUS_RETURN, typename... ZEUS_TYPES>
	zeus::AbstractFunction* createAbstractFunctionDirect(const std::string& _name, ZEUS_RETURN (*_fffp)(ZEUS_TYPES...)) {
		return new AbstractFunctionTypeDirect<ZEUS_RETURN, ZEUS_TYPES...>(_name, _fffp);
	}
}

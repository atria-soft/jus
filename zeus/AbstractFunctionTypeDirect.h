/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.h>
#include <zeus/debug.h>
#include <zeus/AbstractFunction.h>
namespace zeus {
	/**
	 * @brief Execute a call on the global function with a return value
	 * @param[in] _interfaceClient Web interface to send data
	 * @param[in] _obj Buffer input call (that have parameter already check)
	 * @param[in] _func pointer on the function to call
	 */
	template <class ZEUS_RETURN, class... ZEUS_TYPES>
	void executeCall(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
	                 ememory::SharedPtr<zeus::BufferParameter> _obj,
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
		_interfaceClient->addAsync([=](WebServer* _interface) {
			    _interface->answerValue(_obj->getTransactionId(), ret, _obj->getClientId());
			    return true;
			});
	}
	/**
	 * @brief Execute a call on the global function with NO return value
	 * @param[in] _interfaceClient Web interface to send data
	 * @param[in] _obj Buffer input call (that have parameter already check)
	 * @param[in] _func pointer on the function to call
	 */
	template <class... ZEUS_TYPES>
	void executeCall(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
	                 ememory::SharedPtr<zeus::BufferParameter> _obj,
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
		_interfaceClient->addAsync([=](WebServer* _interface) {
			    _interface->answerVoid(_obj->getTransactionId(), _obj->getClientId());
			    return true;
			});
	}
	
	/**
	 * @brief Chass that permit to declare a function that call global function
	 */
	template <class ZEUS_RETURN, class... ZEUS_TYPES>
	class AbstractFunctionTypeDirect: public zeus::AbstractFunction {
		protected:
			static const ParamType m_returnType;
			static const ParamType m_paramType[sizeof...(ZEUS_TYPES)];
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
			             ememory::SharedPtr<zeus::BufferCall> _obj,
			             void* _class) override {
				if (_obj == nullptr) {
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
					                              "WRONG-PARAMETER-NUMBER",
					                              help,
					                              _obj->getClientId());
					return;
				}
				// check parameter compatibility
				for (size_t iii=0; iii<sizeof...(ZEUS_TYPES); ++iii) {
					if (checkCompatibility(m_paramType[iii], _obj->getParameterType(iii)) == false) {
						_interfaceClient->answerError(_obj->getTransactionId(),
						                              "WRONG-PARAMETER-TYPE",
						                              std::string("Parameter id ") + etk::to_string(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'",
						                              _obj->getClientId());
						return;
					}
				}
				// execute cmd:
				zeus::executeCall(_interfaceClient, _obj, m_function);
			}
	};
	// specialization
	template <class ZEUS_RETURN, class... ZEUS_TYPES>
	const ParamType AbstractFunctionTypeDirect<ZEUS_RETURN, ZEUS_TYPES...>::m_returnType = createType<ZEUS_RETURN>();
	// specialization
	template <class ZEUS_RETURN, class... ZEUS_TYPES>
	const ParamType AbstractFunctionTypeDirect<ZEUS_RETURN, ZEUS_TYPES...>::m_paramType[sizeof...(ZEUS_TYPES)] = {createType<ZEUS_TYPES>()...};
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

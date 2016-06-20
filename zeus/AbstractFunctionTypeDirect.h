/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <eproperty/Value.h>
#include <jus/debug.h>
#include <jus/AbstractFunction.h>
namespace jus {
	template <class JUS_RETURN, class... JUS_TYPES>
	void executeCall(const ememory::SharedPtr<jus::TcpString>& _interfaceClient,
	                 uint64_t _transactionId,
	                 uint64_t _clientId,
	                 JUS_RETURN (*_func)(JUS_TYPES...),
	                 jus::Buffer& _obj) {
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			JUS_RETURN ret = _func(_obj.getParameter<JUS_TYPES>(idParam++)...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(JUS_TYPES))-1;
			JUS_RETURN ret = _func(_obj.getParameter<JUS_TYPES>(idParam--)...);
		#else
			#error Must be implemented ...
		#endif
		_interfaceClient->addAsync([=](TcpString* _interface) {
			    _interface->answerValue(_transactionId, ret, _clientId);
			    return true;
			});
	}
	
	template <class... JUS_TYPES>
	void executeCall(const ememory::SharedPtr<jus::TcpString>& _interfaceClient,
	                 uint64_t _transactionId,
	                 uint64_t _clientId,
	                 void (*_func)(JUS_TYPES...),
	                 jus::Buffer& _obj) {
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			_func(_obj.getParameter<JUS_TYPES>(idParam++)...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(JUS_TYPES))-1;
			_func(_obj.getParameter<JUS_TYPES>(idParam--)...);
		#else
			#error Must be implemented ...
		#endif
		_interfaceClient->addAsync([=](TcpString* _interface) {
			    _interface->answerVoid(_transactionId, _clientId);
			    return true;
			});
	}
	
	template <class JUS_RETURN, class... JUS_TYPES>
	class AbstractFunctionTypeDirect: public jus::AbstractFunction {
		protected:
			static const ParamType m_returnType;
			static const ParamType m_paramType[sizeof...(JUS_TYPES)];
		public:
			using functionType = JUS_RETURN (*)(JUS_TYPES...);
			functionType m_function;
			AbstractFunctionTypeDirect(const std::string& _name, const std::string& _desc, functionType _fptr):
			  AbstractFunction(_name, _desc),
			  m_function(_fptr) {
			}
			std::string getPrototype() const override {
				std::string ret;
				ret += m_returnType.getName();
				ret += " ";
				ret += m_name;
				ret += "(";
				for (size_t iii=0; iii<sizeof...(JUS_TYPES); ++iii) {
					if (iii != 0) {
						ret += ", ";
					}
					ret += m_paramType[iii].getName();
				}
				ret += ");";
				return ret;
			}
			std::string getPrototypeReturn() const override {
				return m_returnType.getName();
			}
			std::vector<std::string> getPrototypeParam() const override {
				std::vector<std::string> out;
				for (size_t iii=0; iii<sizeof...(JUS_TYPES); ++iii) {
					out.push_back(m_paramType[iii].getName());
				}
				return out;
			}
			void execute(const ememory::SharedPtr<jus::TcpString>& _interfaceClient,
			             uint64_t _transactionId,
			             uint64_t _clientId,
			             jus::Buffer& _obj,
			             void* _class) override {
				// check parameter number
				if (_obj.getNumberParameter() != sizeof...(JUS_TYPES)) {
					std::string help = "request ";
					help += etk::to_string(_obj.getNumberParameter());
					help += " parameters and need ";
					help += etk::to_string(sizeof...(JUS_TYPES));
					help += " parameters. prototype function:";
					help += getPrototype();
					_interfaceClient->answerError(_transactionId,
					                              "WRONG-PARAMETER-NUMBER",
					                              help,
					                              _clientId);
					return;
				}
				// check parameter compatibility
				for (size_t iii=0; iii<sizeof...(JUS_TYPES); ++iii) {
					if (checkCompatibility(m_paramType[iii], _obj.getParameterType(iii)) == false) {
						_interfaceClient->answerError(_transactionId,
						                              "WRONG-PARAMETER-TYPE",
						                              std::string("Parameter id ") + etk::to_string(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'",
						                              _clientId);
						return;
					}
				}
				// execute cmd:
				jus::executeCall(_interfaceClient, _transactionId, _clientId, m_function, _obj);
			}
	};
	
	template <class JUS_RETURN, class... JUS_TYPES>
	const ParamType AbstractFunctionTypeDirect<JUS_RETURN, JUS_TYPES...>::m_returnType = createType<JUS_RETURN>();
	
	template <class JUS_RETURN, class... JUS_TYPES>
	const ParamType AbstractFunctionTypeDirect<JUS_RETURN, JUS_TYPES...>::m_paramType[sizeof...(JUS_TYPES)] = {createType<JUS_TYPES>()...};
	
	
	template <typename JUS_RETURN, typename... JUS_TYPES>
	AbstractFunction* createAbstractFunctionDirect(const std::string& _name, const std::string& _desc, JUS_RETURN (*_fffp)(JUS_TYPES...)) {
		return new AbstractFunctionTypeDirect<JUS_RETURN, JUS_TYPES...>(_name, _desc, _fffp);
	}
}

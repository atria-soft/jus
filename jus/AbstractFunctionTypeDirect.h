/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <eproperty/Value.h>
#include <ejson/ejson.h>
#include <jus/debug.h>
#include <jus/AbstractFunction.h>
namespace jus {
	template <class JUS_RETURN, class... JUS_TYPES>
	void executeCallJson(const ememory::SharedPtr<jus::TcpString>& _interfaceClient,
	                     uint64_t _transactionId,
	                     uint64_t _clientId,
	                     JUS_RETURN (*_func)(JUS_TYPES...),
	                     const ejson::Array& _params) {
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			ejson::Value ret = jus::convertToJson(_func((jus::convertJsonTo<JUS_TYPES>(_params[idParam++]))...));
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(JUS_TYPES))-1;
			ejson::Value ret = jus::convertToJson(_func(jus::convertJsonTo<JUS_TYPES>(_params[idParam--])...));
		#else
			#error Must be implemented ...
			ejson::Value ret = ejson::Null();
		#endif
		ejson::Object answer;
		answer.add("id", ejson::Number(_transactionId));
		answer.add("client-id", ejson::Number(_clientId));
		answer.add("return", ret);
		JUS_INFO("Answer: " << answer.generateHumanString());
		_interfaceClient->write(answer.generateMachineString());
	}
	
	template <class... JUS_TYPES>
	void executeCallJson(const ememory::SharedPtr<jus::TcpString>& _interfaceClient,
	                     uint64_t _transactionId,
	                     uint64_t _clientId,
	                     void (*_func)(JUS_TYPES...),
	                     const ejson::Array& _params) {
		ejson::Object out;
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			_func((jus::convertJsonTo<JUS_TYPES>(_params[idParam++]))...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(JUS_TYPES))-1;
			_func(jus::convertJsonTo<JUS_TYPES>(_params[idParam--])...);
		#else
			#error Must be implemented ...
		#endif
		_interfaceClient->addAsync([=](TcpString* _interface) {
			ejson::Object answer;
			answer.add("id", ejson::Number(_transactionId));
			answer.add("client-id", ejson::Number(_clientId));
			answer.add("return", ejson::Null());
			JUS_INFO("Answer: " << answer.generateHumanString());
			_interface->write(answer.generateMachineString());
			return true;
			});
	}
	
	template <class JUS_RETURN, class... JUS_TYPES>
	std::string executeCallString(JUS_RETURN (*_func)(JUS_TYPES...), const std::vector<std::string>& _params) {
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			return etk::to_string(_func((jus::convertStringTo<JUS_TYPES>(_params[idParam++]))...));
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(JUS_TYPES))-1;
			return etk::to_string(_func(jus::convertStringTo<JUS_TYPES>(_params[idParam--])...));
		#else
			#error Must be implemented ...
		#endif
		return "";
	}
	template <class... JUS_TYPES>
	std::string executeCallString(void (*_func)(JUS_TYPES...), const std::vector<std::string>& _params) {
		ejson::Object out;
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			_func((jus::convertStringTo<JUS_TYPES>(_params[idParam++]))...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(JUS_TYPES))-1;
			_func(jus::convertStringTo<JUS_TYPES>(_params[idParam--])...);
		#else
			#error Must be implemented ...
		#endif
		return "";
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
			void executeJson(const ememory::SharedPtr<jus::TcpString>& _interfaceClient,
			                 uint64_t _transactionId,
			                 uint64_t _clientId,
			                 const ejson::Array& _params,
			                 void* _class) override {
				// check parameter number
				if (_params.size() != sizeof...(JUS_TYPES)) {
					ejson::Object answer;
					answer.add("id", ejson::Number(_transactionId));
					answer.add("client-id", ejson::Number(_clientId));
					answer.add("error", ejson::String("WRONG-PARAMETER-NUMBER"));
					std::string help = "request ";
					help += etk::to_string(_params.size());
					help += " parameters and need ";
					help += etk::to_string(sizeof...(JUS_TYPES));
					help += " parameters. prototype function:";
					help += getPrototype();
					answer.add("error-help", ejson::String(help));
					JUS_INFO("Answer: " << answer.generateHumanString());
					_interfaceClient->write(answer.generateMachineString());
					return;
				}
				// check parameter compatibility
				for (size_t iii=0; iii<sizeof...(JUS_TYPES); ++iii) {
					if (checkCompatibility(m_paramType[iii], _params[iii]) == false) {
						ejson::Object answer;
						answer.add("id", ejson::Number(_transactionId));
						answer.add("client-id", ejson::Number(_clientId));
						answer.add("error", ejson::String("WRONG-PARAMETER-TYPE"));
						answer.add("error-help", ejson::String("Parameter id " + etk::to_string(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'"));
						JUS_INFO("Answer: " << answer.generateHumanString());
						_interfaceClient->write(answer.generateMachineString());
						return;
					}
				}
				// execute cmd:
				jus::executeCallJson(_interfaceClient, _transactionId, _clientId, m_function, _params);
			}
			std::string executeString(const std::vector<std::string>& _params, void* _class) override {
				std::string out;
				// check parameter number
				if (_params.size() != sizeof...(JUS_TYPES)) {
					JUS_ERROR("Wrong number of Parameters ...");
					out += "error:WRONG-PARAMETER-NUMBER;";
					out += "error-help:request ";
					out += etk::to_string(_params.size());
					out += " parameters and need ";
					out += etk::to_string(sizeof...(JUS_TYPES));
					out += " parameters. prototype function:";
					out += getPrototype();
					return out;
				}
				// check parameter compatibility
				for (size_t iii=0; iii<sizeof...(JUS_TYPES); ++iii) {
					if (checkCompatibility(m_paramType[iii], _params[iii]) == false) {
						out += "error:WRONG-PARAMETER-TYPE;";
						out += "error-help:Parameter id " + etk::to_string(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'";
						return out;
					}
				}
				// execute cmd:
				out = jus::executeCallString(m_function, _params);
				return out;
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

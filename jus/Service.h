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

class ParamType {
	protected:
		const char* m_typeName;
	public:
		ParamType(const char* _name = ""):
		  m_typeName(_name) {
			
		}
		const char* getName() const {
			return m_typeName;
		}
		bool operator == (const ParamType& _obj) const {
			return m_typeName == _obj.m_typeName;
		}
};

template<class JUS_TYPE>
ParamType createType();

#define generate_basic_type(_type, _name) \
template<> ParamType createType<_type>() {\
	return ParamType(_name); \
}

class CmdBase {
	protected:
		const std::string m_name;
	public:
		const std::string& getName() const {
			return m_name;
		}
	protected:
		const std::string m_description;
	public:
		const std::string& getDescription() const {
			return m_description;
		}
	protected:
		CmdBase(const std::string& _name,
		        const std::string& _desc):
		  m_name(_name),
		  m_description(_desc) {
			
		}
	public:
		virtual ~CmdBase() {};
		bool checkCompatibility(const ParamType& _type, const ejson::Value& _params);
		bool checkCompatibility(const ParamType& _type, const std::string& _params);
	public:
		virtual std::string getPrototype() const = 0;
		virtual ejson::Value executeJson(const ejson::Array& _params) = 0;
		virtual std::string executeString(const std::vector<std::string>& _params) = 0;
};

template<class JUS_TYPE>
JUS_TYPE convertStringTo(const std::string& _value);

template<class JUS_TYPE>
JUS_TYPE convertJsonTo(const ejson::Value& _value);

template<class JUS_TYPE>
ejson::Value convertToJson(const JUS_TYPE& _value);

template <class, class...>
class TypeList;

template <class JUS_RETURN, class... JUS_TYPES>
ejson::Value executeCallJson(JUS_RETURN (*_func)(JUS_TYPES...), const ejson::Array& _params) {
	#if defined(__clang__)
		// clang generate a basic warning:
		//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
		int32_t idParam = 0;
		return convertToJson(_func((convertJsonTo<JUS_TYPES>(_params[idParam++]))...));
	#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
		int32_t idParam = m_paramCount-1;
		return convertToJson(_func(convertJsonTo<JUS_TYPES>(_params[idParam--])...));
	#else
		#error Must be implemented ...
	#endif
	return ejson::Null();
}

template <class... JUS_TYPES>
ejson::Value executeCallJson(void (*_func)(JUS_TYPES...), const ejson::Array& _params) {
	ejson::Object out;
	#if defined(__clang__)
		// clang generate a basic warning:
		//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
		int32_t idParam = 0;
		_func((convertJsonTo<JUS_TYPES>(_params[idParam++]))...);
	#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
		int32_t idParam = m_paramCount-1;
		_func(convertJsonTo<JUS_TYPES>(_params[idParam--])...);
	#else
		#error Must be implemented ...
	#endif
	return ejson::Null();
}

template <class JUS_RETURN, class... JUS_TYPES>
std::string executeCallString(JUS_RETURN (*_func)(JUS_TYPES...), const std::vector<std::string>& _params) {
	#if defined(__clang__)
		// clang generate a basic warning:
		//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
		int32_t idParam = 0;
		return etk::to_string(_func((convertStringTo<JUS_TYPES>(_params[idParam++]))...));
	#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
		int32_t idParam = m_paramCount-1;
		return etk::to_string(_func(convertStringTo<JUS_TYPES>(_params[idParam--])...));
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
		_func((convertStringTo<JUS_TYPES>(_params[idParam++]))...);
	#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
		int32_t idParam = m_paramCount-1;
		_func(convertStringTo<JUS_TYPES>(_params[idParam--])...);
	#else
		#error Must be implemented ...
	#endif
	return "";
}

template <class JUS_RETURN, class... JUS_TYPES>
class TypeList: public CmdBase {
	protected:
		static const ParamType m_returnType;
		static const ParamType m_paramType[sizeof...(JUS_TYPES)];
		static const int32_t m_paramCount;
	public:
		using functionType = JUS_RETURN (*)(JUS_TYPES...);
		functionType m_function;
		TypeList(const std::string& _name, const std::string& _desc, functionType _fptr):
		  CmdBase(_name, _desc),
		  m_function(_fptr) {
		}
		std::string getPrototype() const override {
			std::string ret;
			ret += m_returnType.getName();
			ret += " ";
			ret += m_name;
			ret += "(";
			for (size_t iii=0; iii<m_paramCount; ++iii) {
				if (iii != 0) {
					ret += ", ";
				}
				ret += m_paramType[iii].getName();
			}
			ret += ");";
			return ret;
		}
		ejson::Value executeJson(const ejson::Array& _params) override {
			ejson::Object out;
			// check parameter number
			if (_params.size() != m_paramCount) {
				JUS_ERROR("Wrong number of Parameters ...");
				out.add("error", ejson::String("WRONG-PARAMETER-NUMBER"));
				std::string help = "request ";
				help += etk::to_string(_params.size());
				help += " parameters and need ";
				help += etk::to_string(m_paramCount);
				help += " parameters. prototype function:";
				help += getPrototype();
				out.add("error-help", ejson::String(help));
				return out;
			}
			// check parameter compatibility
			for (size_t iii=0; iii<m_paramCount; ++iii) {
				if (checkCompatibility(m_paramType[iii], _params[iii]) == false) {
					out.add("error", ejson::String("WRONG-PARAMETER-TYPE"));
					out.add("error-help", ejson::String("Parameter id " + etk::to_string(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'"));
					return out;
				}
			}
			// execute cmd:
			ejson::Value retVal = executeCallJson(m_function, _params);
			out.add("return", retVal);
			return out;
		}
		std::string executeString(const std::vector<std::string>& _params) override {
			std::string out;
			// check parameter number
			if (_params.size() != m_paramCount) {
				JUS_ERROR("Wrong number of Parameters ...");
				out += "error:WRONG-PARAMETER-NUMBER;";
				out += "error-help:request ";
				out += etk::to_string(_params.size());
				out += " parameters and need ";
				out += etk::to_string(m_paramCount);
				out += " parameters. prototype function:";
				out += getPrototype();
				return out;
			}
			// check parameter compatibility
			for (size_t iii=0; iii<m_paramCount; ++iii) {
				if (checkCompatibility(m_paramType[iii], _params[iii]) == false) {
					out += "error:WRONG-PARAMETER-TYPE;";
					out += "error-help:Parameter id " + etk::to_string(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'";
					return out;
				}
			}
			// execute cmd:
			out = executeCallString(m_function, _params);
			return out;
		}
};

template <class JUS_RETURN, class... JUS_TYPES>
const ParamType TypeList<JUS_RETURN, JUS_TYPES...>::m_returnType = createType<JUS_RETURN>();

template <class JUS_RETURN, class... JUS_TYPES>
const ParamType TypeList<JUS_RETURN, JUS_TYPES...>::m_paramType[sizeof...(JUS_TYPES)] = {createType<JUS_TYPES>()...};

template <class JUS_RETURN, class... JUS_TYPES>
const int32_t TypeList<JUS_RETURN, JUS_TYPES...>::m_paramCount = sizeof...(JUS_TYPES);


template <typename JUS_RETURN, typename... JUS_TYPES>
CmdBase* createCmd(const std::string& _name, const std::string& _desc, JUS_RETURN (*_fffp)(JUS_TYPES...)) {
	return new TypeList<JUS_RETURN, JUS_TYPES...>(_name, _desc, _fffp);
}



static double mulllll(double _val1) {
	double _val2 = 2.0f;
	JUS_ERROR("Call with parameter : " << _val1);
	return _val1*_val2;
}

static void mulllll2(std::string _value, int32_t _val1) {
	JUS_ERROR("Call with parameter : " << _value);
	JUS_ERROR("          parameter : " << _val1);
}

static std::string mulllll3(float _value, bool _val1) {
	JUS_ERROR("Call with parameter : " << _value);
	JUS_ERROR("          parameter : " << _val1);
	double _val2 = 1.0f;
	return "'il fait beau aujoud'hui ...'";
}

static bool mulllll4() {
	return false;
}
namespace jus {
	class Service : public eproperty::Interface {
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
		private:
			jus::TcpString m_interfaceClient;
			uint32_t m_id;
			esignal::Connection m_dataCallback;
			std::vector<std::string> m_newData;
		public:
			Service();
			virtual ~Service();
			// Genenric function call:
			ejson::Object callJson(const ejson::Object& _obj);
			void connect();
			void disconnect();
		private:
			void onClientData(const std::string& _value);
			std::string asyncRead();
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
		protected:
			
			void createSignatureInternal(std::vector<std::string>& _signature) {
				// Finish recursive parse ...
			}
			template<class... _ARGS>
			void createSignatureInternal(std::vector<std::string>& _signature, const std::string& _param, _ARGS&&... _args) {
				_signature.push_back("string");
				createSignatureInternal(_signature, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createSignatureInternal(std::vector<std::string>& _signature, const bool& _param, _ARGS&&... _args) {
				_signature.push_back("bool");
				createSignatureInternal(_signature, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createSignatureInternal(std::vector<std::string>& _signature, const double& _param, _ARGS&&... _args) {
				_signature.push_back("double");
				createSignatureInternal(_signature, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createSignatureInternal(std::vector<std::string>& _signature, const int32_t& _param, _ARGS&&... _args) {
				_signature.push_back("int32");
				createSignatureInternal(_signature, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			std::vector<std::string> createSignature(_ARGS&&... _args) {
				std::vector<std::string> signature;
				createSignatureInternal(signature, std::forward<_ARGS>(_args)...);
				return signature;
			}
			
			template<class JUS_RETURN_VALUE,
			         class JUS_CLASS_TYPE,
			         class... JUS_FUNC_ARGS_TYPE>
			void advertise(const std::string& _name,
			               JUS_RETURN_VALUE (JUS_CLASS_TYPE::*_func)(const JUS_FUNC_ARGS_TYPE&... _args),
			               const std::string& _desc) {
				
				//CmdBase* tmp = createCmd(_name, &mulllll);
				CmdBase* tmp = createCmd(_name, "desc", &mulllll);
				JUS_ERROR("Signature : " << tmp->getPrototype());
				{
					ejson::Array param;
					param.add(ejson::Number(58.5));
					ejson::Value out = tmp->executeJson(param);
					JUS_ERROR("    return: ");
					out.display();
				}
				
				
				tmp = createCmd(_name, "desc", &mulllll2);
				JUS_ERROR("Signature2 : " << tmp->getPrototype());
				{
					ejson::Array param;
					param.add(ejson::String("coucou"));
					param.add(ejson::Number(1563));
					ejson::Value out = tmp->executeJson(param);
					JUS_ERROR("    return: ");
					out.display();
				}
				tmp = createCmd(_name, "desc", &mulllll3);
				JUS_ERROR("Signature3 : " << tmp->getPrototype());
				JUS_ERROR("    return: " << tmp->executeString(etk::split("3.5 false", ' ')));
				
				tmp = createCmd(_name, "desc", &mulllll4);
				JUS_ERROR("Signature4 : " << tmp->getPrototype());
				/*
				std::vector<std::string> plop = createSignature(_args):
				JUS_ERROR("signature:");
				for (auto& it : plop) {
					JUS_ERROR("    - " << it);
				}
				*/
			}
	};
}


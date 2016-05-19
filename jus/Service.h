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
};

template<class JUS_TYPE>
ParamType createType();

#define generate_basic_type(_type, _name) \
template<> ParamType createType<_type>() {\
	return ParamType(_name); \
}

class CmdBase {
	public:
		template<typename JUS_TYPE>
		class Type2String {
			public:
				//std::string operator()();
				static std::string name();
		};
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
		ParamType m_returnType;
		std::vector<ParamType> m_listParamType;
	protected:
		CmdBase(const std::string& _name,
		        const std::string& _desc,
		        const ParamType& _retType,
		        const std::vector<ParamType>& _params):
		  m_name(_name),
		  m_description(_desc),
		  m_returnType(_retType),
		  m_listParamType(_params) {
			
		}
	public:
		virtual ~CmdBase() {};
		//virtual bool checkArguments(const std::vector<CmdBase::Variant>& _params) = 0;
	public:
		std::string getPrototype() const {
			std::string ret;
			ret += m_returnType.getName();
			ret += " ";
			ret += m_name;
			ret += "(";
			for (size_t iii=0; iii<m_listParamType.size(); ++iii) {
				if (iii != 0) {
					ret += ", ";
				}
				ret += m_listParamType[iii].getName();
			}
			ret += ");";
			return ret;
		}
		virtual ejson::Value execute(const ejson::Array& _params) = 0;
};

template<class JUS_TYPE>
JUS_TYPE convertJsonTo(const ejson::Value& _value);

template<class JUS_TYPE>
ejson::Value convertToJson(const JUS_TYPE& _value);

template <class JUS_RETURN, class... JUS_TYPES>
class TypeList: public CmdBase {
	public:
		using functionType = JUS_RETURN (*)(JUS_TYPES...);
		functionType m_function;
		TypeList(const std::string& _name, const std::string& _desc, functionType _fptr):
		  CmdBase(_name, _desc, createType<JUS_RETURN>(), {createType<JUS_TYPES>()...}),
		  //m_sizeParam(sizeof...(JUS_TYPES)),
		  m_function(_fptr) {
			
		}
		ejson::Value execute(const ejson::Array& _params) {
			ejson::Object out;
			if (_params.size() != m_listParamType.size()) {
				JUS_ERROR("Wrong number of Parameters ...");
				out.add("error", ejson::String("WRONG-PARAMETER-NUMBER"));
				std::string help = "request ";
				help += etk::to_string(_params.size());
				help += " parameters and need ";
				help += etk::to_string(m_listParamType.size());
				help += " parameters. prototype function:";
				help += getPrototype();
				out.add("error-help", ejson::String(help));
				return out;
			}
			// TODO : Check params ...
			// Clang and Gcc does not exapnd variadic template at the same way ...
			#if defined(__clang__)
				// clang generate a basic warning:
				//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
				int32_t idParam = 0;
				ejson::Value retVal = convertToJson(m_function(convertJsonTo<JUS_TYPES>(_params[idParam++])...));
			#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
				int32_t idParam = m_listParamType.size()-1;
				ejson::Value retVal = convertToJson(m_function(convertJsonTo<JUS_TYPES>(_params[idParam--])...));
			#else
				#error Must be implemented ...
			#endif
			out.add("return", retVal);
			return out;
		}
};
// Void special case:
template <class... JUS_TYPES>
class TypeListVoid: public CmdBase {
	public:
		using functionType = void (*)(JUS_TYPES...);
		functionType m_function;
		TypeListVoid(const std::string& _name, const std::string& _desc, functionType _fptr):
		  CmdBase(_name, _desc, createType<void>(), {createType<JUS_TYPES>()...}),
		  //m_sizeParam(sizeof...(JUS_TYPES)),
		  m_function(_fptr) {
			
		}
		ejson::Value execute(const ejson::Array& _params) override {
			ejson::Object out;
			if (_params.size() != m_listParamType.size()) {
				JUS_ERROR("Wrong number of Parameters ...");
				out.add("error", ejson::String("WRONG-PARAMETER-NUMBER"));
				std::string help = "request ";
				help += etk::to_string(_params.size());
				help += " parameters and need ";
				help += etk::to_string(m_listParamType.size());
				help += " parameters. prototype function:";
				help += getPrototype();
				out.add("error-help", ejson::String(help));
				return out;
			}
			// TODO : Check params ...
			// Clang and Gcc does not exapnd variadic template at the same way ...
			#if defined(__clang__)
				// clang generate a basic warning:
				//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
				int32_t idParam = 0;
				m_function(convertJsonTo<JUS_TYPES>(_params[idParam++])...);
			#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
				int32_t idParam = m_listParamType.size()-1;
				m_function(convertJsonTo<JUS_TYPES>(_params[idParam--])...);
			#else
				#error Must be implemented ...
			#endif
			out.add("return", ejson::Null());
			return out;
		}
};

template <typename JUS_RETURN, typename... JUS_TYPES>
CmdBase* createCmd(const std::string& _name, const std::string& _desc, JUS_RETURN (*_fffp)(JUS_TYPES...)) {
	return new TypeList<JUS_RETURN, JUS_TYPES...>(_name, _desc, _fffp);
}
template <typename... JUS_TYPES>
CmdBase* createCmd(const std::string& _name, const std::string& _desc, void (*_fffp)(JUS_TYPES...)) {
	return new TypeListVoid<JUS_TYPES...>(_name, _desc, _fffp);
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
	double _val2 = 1.0f;
	return "";
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
					ejson::Value out = tmp->execute(param);
					JUS_ERROR("    return: ");
					out.display();
				}
				
				
				tmp = createCmd(_name, "desc", &mulllll2);
				JUS_ERROR("Signature2 : " << tmp->getPrototype());
				{
					ejson::Array param;
					param.add(ejson::String("coucou"));
					param.add(ejson::Number(1563));
					ejson::Value out = tmp->execute(param);
					JUS_ERROR("    return: ");
					out.display();
				}
				tmp = createCmd(_name, "desc", &mulllll3);
				JUS_ERROR("Signature3 : " << tmp->getPrototype());
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


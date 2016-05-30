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
#include <jus/ParamType.h>
namespace jus {
	class AbstractFunction {
		public:
			enum class type {
				unknow,
				global,
				local,
				service,
				object,
			};
		protected:
			enum type m_type;
		public:
			enum type getType() const;
			void setType(enum type _type);
		protected:
			std::string m_name;
		public:
			const std::string& getName() const;
		protected:
			std::string m_description;
		public:
			const std::string& getDescription() const;
			void setDescription(const std::string& _desc);
		protected:
			std::vector<std::pair<std::string, std::string>> m_paramsDescription;
		public:
			void setParam(int32_t _idParam, const std::string& _name, const std::string& _desc);
			void addParam(const std::string& _name, const std::string& _desc);
		protected:
			std::string m_returnDescription;
		public:
			void setReturn(const std::string& _desc);
		protected:
			AbstractFunction(const std::string& _name, const std::string& _desc="");
		public:
			virtual ~AbstractFunction() {};
		public:
			bool checkCompatibility(const ParamType& _type, const ejson::Value& _params);
			bool checkCompatibility(const ParamType& _type, const std::string& _params);
		public:
			std::string getPrototypeFull() const;
			virtual std::string getPrototype() const = 0;
			virtual std::string getPrototypeReturn() const = 0;
			virtual std::vector<std::string> getPrototypeParam() const = 0;
			virtual ejson::Value executeJson(const ejson::Array& _params, void* _class=nullptr) = 0;
			virtual std::string executeString(const std::vector<std::string>& _params, void* _class=nullptr) = 0;
	};
	
	template<class JUS_TYPE>
	JUS_TYPE convertStringTo(const std::string& _value);
	
	template<class JUS_TYPE>
	JUS_TYPE convertJsonTo(const ejson::Value& _value);
	
	template<class JUS_TYPE>
	ejson::Value convertToJson(const JUS_TYPE& _value);
	
	
	ejson::Object createBaseCall(uint64_t _transactionId, const std::string& _functionName, const std::string& _service="");
	
	void createParam(ejson::Object& _obj);
	
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const char* _param, _ARGS&&... _args);
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::string& _param, _ARGS&&... _args);
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const bool& _param, _ARGS&&... _args);
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const int32_t& _param, _ARGS&&... _args);
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const double& _param, _ARGS&&... _args);
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const float& _param, _ARGS&&... _args);
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::vector<std::string>& _param, _ARGS&&... _args);
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::vector<bool>& _param, _ARGS&&... _args);
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::vector<int32_t>& _param, _ARGS&&... _args);
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::vector<double>& _param, _ARGS&&... _args);
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::vector<float>& _param, _ARGS&&... _args);
	
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const char* _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		if (_param == nullptr) {
			array.add(ejson::String());
		} else {
			array.add(ejson::String(_param));
		}
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::string& _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		array.add(ejson::String(_param));
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const bool& _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		array.add(ejson::Boolean(_param));
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const int32_t& _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		array.add(ejson::Number(_param));
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const double& _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		array.add(ejson::Number(_param));
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const float& _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		array.add(ejson::Number(_param));
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::vector<std::string>& _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		ejson::Array array2;
		for (auto& it : _param) {
			array2.add(ejson::String(it));
		}
		array.add(array2);
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::vector<bool>& _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		ejson::Array array2;
		for (const auto& it : _param) {
			array2.add(ejson::Boolean(it));
		}
		array.add(array2);
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::vector<int32_t>& _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		ejson::Array array2;
		for (auto& it : _param) {
			array2.add(ejson::Number(it));
		}
		array.add(array2);
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::vector<double>& _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		ejson::Array array2;
		for (auto& it : _param) {
			array2.add(ejson::Number(it));
		}
		array.add(array2);
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	template<class... _ARGS>
	void createParam(ejson::Object& _obj, const std::vector<float>& _param, _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		ejson::Array array2;
		for (auto& it : _param) {
			array2.add(ejson::Number(it));
		}
		array.add(array2);
		createParam(_obj, std::forward<_ARGS>(_args)...);
	}
	
	template<class... _ARGS>
	ejson::Object createCall(uint64_t _transactionId, const std::string& _functionName, _ARGS&&... _args) {
		ejson::Object callElem = createBaseCall(_transactionId, _functionName);
		createParam(callElem, std::forward<_ARGS>(_args)...);
		return callElem;
	}
	template<class... _ARGS>
	ejson::Object createCallService(uint64_t _transactionId, const std::string& _serviceName, const std::string& _functionName, _ARGS&&... _args) {
		ejson::Object callElem = createBaseCall(_transactionId, _functionName, _serviceName);
		createParam(callElem, std::forward<_ARGS>(_args)...);
		return callElem;
	}
	ejson::Object createCallJson(uint64_t _transactionId, const std::string& _functionName, ejson::Array _params);
	
}


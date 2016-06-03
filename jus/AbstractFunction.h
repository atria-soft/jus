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
#include <jus/File.h>
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
			virtual void executeJson(const ememory::SharedPtr<jus::TcpString>& _interfaceClient, uint64_t _transactionId, uint64_t _clientId, const ejson::Array& _params, void* _class=nullptr) = 0;
			virtual std::string executeString(const std::vector<std::string>& _params, void* _class=nullptr) = 0;
	};
	
	// define basic async call element ...
	using ActionAsyncClient = std::function<bool(TcpString* _interface, const std::string& _service, uint64_t _transactionId, uint64_t _part)>;
	
	template<class JUS_TYPE>
	JUS_TYPE convertStringTo(const std::string& _value);
	
	template<class JUS_TYPE>
	JUS_TYPE convertJsonTo(const ejson::Value& _value);
	
	template<class JUS_TYPE>
	ejson::Value convertToJson(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const JUS_TYPE& _value);
	//ejson::Value convertToJson(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const char* _value);
	
	
	ejson::Object createBaseCall(uint64_t _transactionId, const std::string& _functionName, const std::string& _service="");
	
	void createParam(std::vector<ActionAsyncClient>& _asyncAction,
	                 int32_t _paramId,
	                 ejson::Object& _obj);
	
	template<class JUS_TYPE, class... _ARGS>
	void createParam(std::vector<ActionAsyncClient>& _asyncAction,
	                 int32_t _paramId,
	                 ejson::Object& _obj,
	                 const JUS_TYPE& _param,
	                 _ARGS&&... _args) {
		if (_obj.valueExist("param") == false) {
			_obj.add("param", ejson::Array());
		}
		ejson::Array array = _obj["param"].toArray();
		array.add(convertToJson<JUS_TYPE>(_asyncAction, _paramId, _param));
		_paramId++;
		createParam(_asyncAction, _paramId, _obj, std::forward<_ARGS>(_args)...);
	}
	// convert const char in std::string ...
	template<class... _ARGS>
	void createParam(std::vector<ActionAsyncClient>& _asyncAction,
	                 int32_t _paramId,
	                 ejson::Object& _obj,
	                 const char* _param,
	                 _ARGS&&... _args) {
		createParam(_asyncAction, _paramId, _obj, std::string(_param), std::forward<_ARGS>(_args)...);
	}
	
	
	template<class... _ARGS>
	ejson::Object createCall(std::vector<ActionAsyncClient>& _asyncAction, uint64_t _transactionId, const std::string& _functionName, _ARGS&&... _args) {
		ejson::Object callElem = createBaseCall(_transactionId, _functionName);
		createParam(_asyncAction, 0, callElem, std::forward<_ARGS>(_args)...);
		return callElem;
	}
	template<class... _ARGS>
	ejson::Object createCallService(std::vector<ActionAsyncClient>& _asyncAction, uint64_t _transactionId, const std::string& _serviceName, const std::string& _functionName, _ARGS&&... _args) {
		ejson::Object callElem = createBaseCall(_transactionId, _functionName, _serviceName);
		createParam(_asyncAction, 0, callElem, std::forward<_ARGS>(_args)...);
		return callElem;
	}
	ejson::Object createCallJson(uint64_t _transactionId, const std::string& _functionName, ejson::Array _params);
	
}


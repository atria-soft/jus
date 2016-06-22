/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <eproperty/Value.h>
#include <zeus/debug.h>
#include <zeus/ParamType.h>
#include <zeus/File.h>
#include <zeus/Buffer.h>
#include <ememory/memory.h>


namespace zeus {
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
			bool checkCompatibility(const ParamType& _type, const ParamType& _params);
		public:
			std::string getPrototypeFull() const;
			virtual std::string getPrototype() const = 0;
			virtual std::string getPrototypeReturn() const = 0;
			virtual std::vector<std::string> getPrototypeParam() const = 0;
			virtual void execute(const ememory::SharedPtr<zeus::TcpString>& _interfaceClient,
			                     uint64_t _transactionId,
			                     uint64_t _clientId,
			                     const ememory::SharedPtr<zeus::Buffer>& _params,
			                     void* _class=nullptr) = 0;
	};
	
	ememory::SharedPtr<zeus::Buffer> createBinaryBaseCall(uint64_t _transactionId, const std::string& _functionName, const uint32_t& _serviceId=0);
	void createBinaryParam(int32_t _paramId,
	                       const ememory::SharedPtr<zeus::Buffer>& _obj);
	
	template<class ZEUS_TYPE, class... _ARGS>
	void createBinaryParam(int32_t _paramId,
	                       const ememory::SharedPtr<zeus::Buffer>& _obj,
	                       const ZEUS_TYPE& _param,
	                       _ARGS&&... _args) {
		_obj->addParameter<ZEUS_TYPE>(_param);
		_paramId++;
		createBinaryParam(_paramId, _obj, std::forward<_ARGS>(_args)...);
	}
	// convert const char in std::string ...
	template<class... _ARGS>
	void createBinaryParam(int32_t _paramId,
	                       const ememory::SharedPtr<zeus::Buffer>& _obj,
	                       const char* _param,
	                       _ARGS&&... _args) {
		createBinaryParam(_paramId, _obj, std::string(_param), std::forward<_ARGS>(_args)...);
	}
	
	template<class... _ARGS>
	ememory::SharedPtr<zeus::Buffer> createBinaryCall(uint64_t _transactionId, const std::string& _functionName, _ARGS&&... _args) {
		ememory::SharedPtr<zeus::Buffer> callElem = createBinaryBaseCall(_transactionId, _functionName);
		if (callElem == nullptr) {
			return nullptr;
		}
		createBinaryParam(0, callElem, std::forward<_ARGS>(_args)...);
		return callElem;
	}
	template<class... _ARGS>
	ememory::SharedPtr<zeus::Buffer> createBinaryCallService(uint64_t _transactionId, const uint32_t& _serviceName, const std::string& _functionName, _ARGS&&... _args) {
		ememory::SharedPtr<zeus::Buffer> callElem = createBinaryBaseCall(_transactionId, _functionName, _serviceName);
		if (callElem == nullptr) {
			return nullptr;
		}
		createBinaryParam(0, callElem, std::forward<_ARGS>(_args)...);
		return callElem;
	}
}

#include <zeus/TcpString.h>


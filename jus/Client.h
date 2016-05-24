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
#include <jus/ServiceRemote.h>

namespace jus {
	class Client : public eproperty::Interface {
		friend class ServiceRemote;
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
		private:
			jus::TcpString m_interfaceClient;
			uint32_t m_id;
			std::vector<std::string> m_newData;
		public:
			Client();
			virtual ~Client();
			void connect(const std::string& _remoteUserToConnect);
			void disconnect();
		public:
			jus::ServiceRemote getService(const std::string& _serviceName);
			void link(const std::string& _serviceName);
			void unlink(const std::string& _serviceName);
		private:
			void onClientData(std::string _value);
			std::string asyncRead();
			ejson::Object callJson(const ejson::Object& _obj);
			ejson::Object createBaseCall(const std::string& _functionName, const std::string& _service="");
			void createParam(ejson::Object& _obj) {
				// Finish recursive parse ...
			}
			template<class... _ARGS>
			void createParam(ejson::Object& _obj, const char* _param, _ARGS&&... _args) {
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
				ejson::Array array = _obj["param"].toArray();
				array.add(ejson::String(_param));
				createParam(_obj, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createParam(ejson::Object& _obj, const bool& _param, _ARGS&&... _args) {
				ejson::Array array = _obj["param"].toArray();
				array.add(ejson::Boolean(_param));
				createParam(_obj, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createParam(ejson::Object& _obj, const int32_t& _param, _ARGS&&... _args) {
				ejson::Array array = _obj["param"].toArray();
				array.add(ejson::Number(_param));
				createParam(_obj, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createParam(ejson::Object& _obj, const double& _param, _ARGS&&... _args) {
				ejson::Array array = _obj["param"].toArray();
				array.add(ejson::Number(_param));
				createParam(_obj, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createParam(ejson::Object& _obj, const float& _param, _ARGS&&... _args) {
				ejson::Array array = _obj["param"].toArray();
				array.add(ejson::Number(_param));
				createParam(_obj, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createParam(ejson::Object& _obj, const std::vector<std::string>& _param, _ARGS&&... _args) {
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
				ejson::Array array = _obj["param"].toArray();
				ejson::Array array2;
				for (auto& it : _param) {
					array2.add(ejson::Number(it));
				}
				array.add(array2);
				createParam(_obj, std::forward<_ARGS>(_args)...);
			}
		public:
			template<class... _ARGS>
			void call(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = createBaseCall(_functionName);
				createParam(callElem, std::forward<_ARGS>(_args)...);
				ejson::Object obj = callJson(callElem);
				if (obj.valueExist("error") == true) {
					JUS_WARNING("call error: " << obj["error"]);
				}
			}
			template<class... _ARGS>
			int32_t call_i(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = createBaseCall(_functionName);
				createParam(callElem, std::forward<_ARGS>(_args)...);
				ejson::Object obj = callJson(callElem);
				if (obj.valueExist("error") == true) {
					JUS_WARNING("call error: " << obj["error"]);
				}
				ejson::Value val = obj["return"];
				if (val.exist() == false) {
					JUS_WARNING("No Return value ...");
					return 0;
				}
				if (val.isNumber() == false) {
					JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
					return 0;
				}
				return int32_t(val.toNumber().get());
			}
			template<class... _ARGS>
			double call_d(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = createBaseCall(_functionName);
				createParam(callElem, std::forward<_ARGS>(_args)...);
				ejson::Object obj = callJson(callElem);
				if (obj.valueExist("error") == true) {
					JUS_WARNING("call error: " << obj["error"]);
				}
				ejson::Value val = obj["return"];
				if (val.exist() == false) {
					JUS_WARNING("No Return value ...");
					return 0.0;
				}
				if (val.isNumber() == false) {
					JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
					return 0.0;
				}
				return val.toNumber().get();
			}
			template<class... _ARGS>
			std::string call_s(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = createBaseCall(_functionName);
				createParam(callElem, std::forward<_ARGS>(_args)...);
				ejson::Object obj = callJson(callElem);
				if (obj.valueExist("error") == true) {
					JUS_WARNING("call error: " << obj["error"]);
				}
				ejson::Value val = obj["return"];
				if (val.exist() == false) {
					JUS_WARNING("No Return value ...");
					return "";
				}
				if (val.isString() == false) {
					JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'String'");
					return "";
				}
				return val.toString().get();
			}
			template<class... _ARGS>
			bool call_b(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = createBaseCall(_functionName);
				createParam(callElem, std::forward<_ARGS>(_args)...);
				ejson::Object obj = callJson(callElem);
				if (obj.valueExist("error") == true) {
					JUS_WARNING("call error: " << obj["error"]);
				}
				ejson::Value val = obj["return"];
				if (val.exist() == false) {
					JUS_WARNING("No Return value ...");
					return false;
				}
				if (val.isBoolean() == false) {
					JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Boolean'");
					return false;
				}
				return val.toBoolean().get();
			}
			template<class... _ARGS>
			std::vector<int32_t> call_vi(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = createBaseCall(_functionName);
				createParam(callElem, std::forward<_ARGS>(_args)...);
				ejson::Object obj = callJson(callElem);
				if (obj.valueExist("error") == true) {
					JUS_WARNING("call error: " << obj["error"]);
				}
				if (obj.valueExist("return") == false) {
					JUS_WARNING("No Return value ...");
					return std::vector<int32_t>();
				}
				ejson::Value val = obj["return"];
				if (val.exist() == false) {
					JUS_WARNING("No Return value ...");
					return std::vector<int32_t>();
				}
				if (val.isArray() == false) {
					JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
					return std::vector<int32_t>();
				}
				std::vector<int32_t> out;
				for (auto it : val.toArray()) {
					if (val.isNumber() == false) {
						JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
						continue;
					}
					out.push_back(int32_t(val.toNumber().get()));
				}
				return out;
			}
			template<class... _ARGS>
			std::vector<double> call_vd(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = createBaseCall(_functionName);
				createParam(callElem, std::forward<_ARGS>(_args)...);
				ejson::Object obj = callJson(callElem);
				if (obj.valueExist("error") == true) {
					JUS_WARNING("call error: " << obj["error"]);
				}
				if (obj.valueExist("return") == false) {
					JUS_WARNING("No Return value ...");
					return std::vector<double>();
				}
				ejson::Value val = obj["return"];
				if (val.exist() == false) {
					JUS_WARNING("No Return value ...");
					return std::vector<double>();
				}
				if (val.isArray() == false) {
					JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
					return std::vector<double>();
				}
				std::vector<double> out;
				for (auto it : val.toArray()) {
					if (it.isNumber() == false) {
						JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
						continue;
					}
					out.push_back(it.toNumber().get());
				}
				return out;
			}
			template<class... _ARGS>
			std::vector<std::string> call_vs(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = createBaseCall(_functionName);
				createParam(callElem, std::forward<_ARGS>(_args)...);
				ejson::Object obj = callJson(callElem);
				if (obj.valueExist("error") == true) {
					JUS_WARNING("call error: " << obj["error"]);
				}
				if (obj.valueExist("return") == false) {
					JUS_WARNING("No Return value ...");
					return std::vector<std::string>();
				}
				ejson::Value val = obj["return"];
				if (val.exist() == false) {
					JUS_WARNING("No Return value ...");
					return std::vector<std::string>();
				}
				if (val.isArray() == false) {
					JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
					return std::vector<std::string>();
				}
				std::vector<std::string> out;
				for (auto it : val.toArray()) {
					if (it.isString() == false) {
						JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'String'");
						continue;
					}
					out.push_back(it.toString().get());
				}
				return out;
			}
			template<class... _ARGS>
			std::vector<bool> call_vb(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = createBaseCall(_functionName);
				createParam(callElem, std::forward<_ARGS>(_args)...);
				ejson::Object obj = callJson(callElem);
				if (obj.valueExist("error") == true) {
					JUS_WARNING("call error: " << obj["error"]);
				}
				if (obj.valueExist("return") == false) {
					JUS_WARNING("No Return value ...");
					return std::vector<bool>();
				}
				ejson::Value val = obj["return"];
				if (val.exist() == false) {
					JUS_WARNING("No Return value ...");
					return std::vector<bool>();
				}
				if (val.isArray() == false) {
					JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
					return std::vector<bool>();
				}
				std::vector<bool> out;
				for (auto it : val.toArray()) {
					if (it.isBoolean() == false) {
						JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Boolean'");
						continue;
					}
					out.push_back(it.toBoolean().get());
				}
				return out;
			}
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
	};
	/*
	template<class RETURN_TYPE>
	RETURN_TYPE Client::call(const std::string& _functionName) {
		ejson::Object callElem = ejson::Object(std::string("{ 'call':'") + _functionName + "'}");
		ejson::Object obj = callJson(callElem);
		return 256;
	}
	template<class RETURN_TYPE>
	RETURN_TYPE Client::call(const std::string& _functionName) {
		ejson::Object callElem = ejson::Object(std::string("{ 'call':'") + _functionName + "'}");
		ejson::Object obj = callJson(callElem);
		return std::vector<std::string>();
	}
	*/
}


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
#include <chrono>
#include <unistd.h>

namespace jus {
	class FutureData {
		public:
			uint64_t m_transactionId;
			bool m_isFinished;
			ejson::Object m_returnData;
	};
	class FutureBase {
		public: // TODO: Do it better ..
			ememory::SharedPtr<jus::FutureData> m_data;
		public:
			FutureBase() {
				m_data = nullptr;
			}
			FutureBase(uint64_t _transactionId) {
				m_data = std::make_shared<jus::FutureData>();
				if (m_data == nullptr) {
					return;
				}
				m_data->m_transactionId = _transactionId;
				m_data->m_isFinished = false;
			}
			FutureBase(uint64_t _transactionId, bool _isFinished, ejson::Object _returnData) {
				m_data = std::make_shared<jus::FutureData>();
				if (m_data == nullptr) {
					return;
				}
				m_data->m_transactionId = _transactionId;
				m_data->m_isFinished = _isFinished;
				m_data->m_returnData = _returnData;
			}
			jus::FutureBase operator= (const jus::FutureBase& _base) {
				m_data = _base.m_data;
				return *this;
			}
			void setAnswer(const ejson::Object& _returnValue) {
				if (m_data == nullptr) {
					JUS_ERROR(" Not a valid future ...");
					return;
				}
				m_data->m_returnData = _returnValue;
				m_data->m_isFinished = true;
			}
			uint64_t getTransactionId() {
				if (m_data == nullptr) {
					return 0;
				}
				return m_data->m_transactionId;
			}
			bool hasError() {
				if (m_data == nullptr) {
					return true;
				}
				return m_data->m_returnData.valueExist("error");
			}
			std::string getErrorType() {
				if (m_data == nullptr) {
					return "NULL_PTR";
				}
				return m_data->m_returnData["error"].toString().get();
			}
			std::string getErrorHelp() {
				if (m_data == nullptr) {
					return "Thsi is a nullptr future";
				}
				return m_data->m_returnData["error-help"].toString().get();
			}
			bool isValid() {
				return m_data != nullptr;
			}
			bool isFinished() {
				if (m_data == nullptr) {
					return true;
				}
				return m_data->m_isFinished;
			}
			void wait() {
				while (isFinished() == false) {
					// TODO : Do it better ... like messaging/mutex_locked ...
					usleep(10000);
				}
			}
			bool waitFor(std::chrono::microseconds _delta) {
				std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
				while (    std::chrono::steady_clock::now() - start < _delta
				        && isFinished() == false) {
					// TODO : Do it better ... like messaging/mutex_locked ...
					usleep(10000);
				}
				return isFinished();
			}
			bool waitUntile(std::chrono::steady_clock::time_point _endTime) {
				while (    std::chrono::steady_clock::now() < _endTime
				        && isFinished() == false) {
					// TODO : Do it better ... like messaging/mutex_locked ...
					usleep(10000);
				}
				return isFinished();
			}
	};
	
	template<class JUS_RETURN>
	class Future : public jus::FutureBase {
		public:
			jus::Future<JUS_RETURN>& operator= (const jus::FutureBase& _base) {
				m_data = _base.m_data;
				return *this;
			}
			JUS_RETURN get();
	};

	class Client : public eproperty::Interface {
		friend class ServiceRemote;
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
			std::mutex m_mutex;
			std::vector<jus::FutureBase> m_pendingCall;
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
			bool link(const std::string& _serviceName);
			void unlink(const std::string& _serviceName);
			
			// Connect that is not us
			//bool identify("clientTest1#atria-soft.com", "QSDQSDGQSF54HSXWVCSQDJ654URTDJ654NBXCDFDGAEZ51968");
			// Connect to ourself:
			//client1.authentificate("coucou");
		private:
			void onClientData(std::string _value);
			std::string asyncRead();
			ejson::Object callJson(const ejson::Object& _obj);
			jus::FutureBase sendJson(uint64_t _transactionId, const ejson::Object& _obj);
		public:
		
			uint64_t getId();
			template<class JUS_RETURN, class... _ARGS>
			jus::Future<JUS_RETURN> callAsync(const std::string& _functionName, _ARGS&&... _args) {
				uint64_t id = getId();
				ejson::Object callElem = jus::createCall(id, _functionName, std::forward<_ARGS>(_args)...);
				jus::Future<JUS_RETURN> out = sendJson(id, callElem);
				return out;
			}
			
			template<class... _ARGS>
			void call(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = jus::createCall(getId(), _functionName, std::forward<_ARGS>(_args)...);
				ejson::Object obj = callJson(callElem);
				if (obj.valueExist("error") == true) {
					JUS_WARNING("call error: " << obj["error"]);
				}
			}
			template<class... _ARGS>
			int32_t call_i(const std::string& _functionName, _ARGS&&... _args) {
				ejson::Object callElem = jus::createCall(getId(), _functionName, std::forward<_ARGS>(_args)...);
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
				ejson::Object callElem = jus::createCall(getId(), _functionName, std::forward<_ARGS>(_args)...);
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
				ejson::Object callElem = jus::createCall(getId(), _functionName, std::forward<_ARGS>(_args)...);
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
				ejson::Object callElem = jus::createCall(getId(), _functionName, std::forward<_ARGS>(_args)...);
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
				ejson::Object callElem = jus::createCall(getId(), _functionName, std::forward<_ARGS>(_args)...);
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
				ejson::Object callElem = jus::createCall(getId(), _functionName, std::forward<_ARGS>(_args)...);
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
				ejson::Object callElem = jus::createCall(getId(), _functionName, std::forward<_ARGS>(_args)...);
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
				ejson::Object callElem = jus::createCall(getId(), _functionName, std::forward<_ARGS>(_args)...);
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


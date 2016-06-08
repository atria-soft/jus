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
#include <jus/ServiceRemote.h>
#include <jus/Future.h>
#include <jus/connectionMode.h>

namespace jus {
	class Client;
	class ServiceRemote {
		private:
			jus::Client* m_clientInterface;
			std::string m_name;
			uint32_t m_serviceId;
			bool m_isLinked;
		public:
			ServiceRemote(jus::Client* _clientInterface, const std::string& _name);
			~ServiceRemote();
			bool exist();
		private:
			jus::FutureBase callJson(uint64_t _transactionId, const ejson::Object& _obj, const std::vector<ActionAsyncClient>& _async, jus::FutureData::ObserverFinish _callback=nullptr);
			jus::FutureBase callBinary(uint64_t _transactionId, const jus::Buffer& _obj, const std::vector<ActionAsyncClient>& _async, jus::FutureData::ObserverFinish _callback=nullptr);
			uint64_t getId();
			enum jus::connectionMode getMode();
		public:
			template<class... _ARGS>
			jus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				uint64_t id = getId();
				std::vector<ActionAsyncClient> asyncActionToDo;
				if (getMode() == jus::connectionMode::modeJson) {
					ejson::Object callElem = jus::createCallService(asyncActionToDo, id, m_serviceId, _functionName, std::forward<_ARGS>(_args)...);
					return callJson(id, callElem, asyncActionToDo);
				} else {
					jus::Buffer callElem = jus::createBinaryCallService(asyncActionToDo, id, m_serviceId, _functionName, std::forward<_ARGS>(_args)...);
					return callBinary(id, callElem, asyncActionToDo);
				}
			}
			template<class... _ARGS>
			jus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args, jus::FutureData::ObserverFinish _callback) {
				uint64_t id = getId();
				std::vector<ActionAsyncClient> asyncActionToDo;
				if (getMode() == jus::connectionMode::modeJson) {
					ejson::Object callElem = jus::createCallService(asyncActionToDo, id, m_serviceId, _functionName, std::forward<_ARGS>(_args)...);
					return callJson(id, callElem, asyncActionToDo, _callback);
				} else {
					jus::Buffer callElem = jus::createBinaryCallService(asyncActionToDo, id, m_serviceId, _functionName, std::forward<_ARGS>(_args)...);
					return callBinary(id, callElem, asyncActionToDo, _callback);
				}
			}
	};
}


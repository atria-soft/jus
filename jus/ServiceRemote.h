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

namespace jus {
	class Client;
	class ServiceRemote {
		private:
			jus::Client* m_clientInterface;
			std::string m_name;
			bool m_isLinked;
		public:
			ServiceRemote(jus::Client* _clientInterface, const std::string& _name);
			~ServiceRemote();
			bool exist();
		private:
			jus::FutureBase callJson(uint64_t _transactionId, const ejson::Object& _obj);
			uint64_t getId();
		public:
			template<class... _ARGS>
			jus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				uint64_t id = getId();
				ejson::Object callElem = jus::createCall(id, _functionName, std::forward<_ARGS>(_args)...);
				return callJson(id, callElem);
			}
	};
}


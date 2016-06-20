/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <eproperty/Value.h>
#include <jus/debug.h>
#include <jus/AbstractFunction.h>
#include <jus/ServiceRemote.h>
#include <jus/Future.h>
#include <jus/TcpString.h>

namespace jus {
	class Client;
	class ServiceRemote {
		private:
			ememory::SharedPtr<jus::TcpString> m_interfaceClient;
			std::string m_name;
			uint32_t m_serviceId;
			bool m_isLinked;
		public:
			ServiceRemote(ememory::SharedPtr<jus::TcpString> _clientLink, const std::string& _name);
			~ServiceRemote();
			bool exist();
		public:
			template<class... _ARGS>
			jus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				if (m_interfaceClient == nullptr) {
					jus::Buffer ret;
					ret.addError("NULLPTR", "call " + _functionName + " with no interface open");
					return jus::FutureBase(0, true, ret);
				}
				return m_interfaceClient->callService(m_serviceId, _functionName, _args...);
			}
			template<class... _ARGS>
			jus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args, jus::FutureData::ObserverFinish _callback) {
				if (m_interfaceClient == nullptr) {
					jus::Buffer ret;
					ret.addError("NULLPTR", "call " + _functionName + " with no interface open");
					return jus::FutureBase(0, true, ret, _callback);
				}
				return m_interfaceClient->callServiceAction(m_serviceId, _functionName, _args..., _callback);
			}
	};
}


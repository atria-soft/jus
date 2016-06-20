/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/TcpString.h>
#include <eproperty/Value.h>
#include <zeus/debug.h>
#include <zeus/AbstractFunction.h>
#include <zeus/ServiceRemote.h>
#include <zeus/Future.h>
#include <zeus/TcpString.h>

namespace zeus {
	class Client;
	class ServiceRemote {
		private:
			ememory::SharedPtr<zeus::TcpString> m_interfaceClient;
			std::string m_name;
			uint32_t m_serviceId;
			bool m_isLinked;
		public:
			ServiceRemote(ememory::SharedPtr<zeus::TcpString> _clientLink, const std::string& _name);
			~ServiceRemote();
			bool exist();
		public:
			template<class... _ARGS>
			zeus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				if (m_interfaceClient == nullptr) {
					zeus::Buffer ret;
					ret.addError("NULLPTR", "call " + _functionName + " with no interface open");
					return zeus::FutureBase(0, true, ret);
				}
				return m_interfaceClient->callService(m_serviceId, _functionName, _args...);
			}
			template<class... _ARGS>
			zeus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args, zeus::FutureData::ObserverFinish _callback) {
				if (m_interfaceClient == nullptr) {
					zeus::Buffer ret;
					ret.addError("NULLPTR", "call " + _functionName + " with no interface open");
					return zeus::FutureBase(0, true, ret, _callback);
				}
				return m_interfaceClient->callServiceAction(m_serviceId, _functionName, _args..., _callback);
			}
	};
}


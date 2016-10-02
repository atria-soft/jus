/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <zeus/debug.hpp>
#include <zeus/AbstractFunction.hpp>
#include <zeus/ServiceRemote.hpp>
#include <zeus/Future.hpp>
#include <zeus/WebServer.hpp>

namespace zeus {
	class Client;
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	class ServiceRemote {
		private:
			ememory::SharedPtr<zeus::WebServer> m_interfaceClient;
			std::string m_name;
			uint32_t m_serviceId;
			bool m_isLinked;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			ServiceRemote(ememory::SharedPtr<zeus::WebServer> _clientLink, const std::string& _name);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			~ServiceRemote();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool exist();
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class... _ARGS>
			zeus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) {
				if (m_interfaceClient == nullptr) {
					ememory::SharedPtr<zeus::BufferAnswer> ret = zeus::BufferAnswer::create();
					if (ret != nullptr) {
						ret->addError("NULLPTR", "call " + _functionName + " with no interface open");
					}
					return zeus::FutureBase(0, ret);
				}
				return m_interfaceClient->callService(m_serviceId, _functionName, _args...);
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class... _ARGS>
			zeus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args, zeus::FutureData::ObserverFinish _callback) {
				if (m_interfaceClient == nullptr) {
					ememory::SharedPtr<zeus::BufferAnswer> ret = zeus::BufferAnswer::create();
					if (ret != nullptr) {
						ret->addError("NULLPTR", "call " + _functionName + " with no interface open");
					}
					return zeus::FutureBase(0, ret, _callback);
				}
				return m_interfaceClient->callServiceAction(m_serviceId, _functionName, _args..., _callback);
			}
	};
}


/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <zeus/AbstractFunctionTypeDirect.hpp>
#include <zeus/AbstractFunctionTypeClass.hpp>
#include <zeus/debug.hpp>
#include <zeus/WebObj.hpp>

namespace zeus {
	/**
	 * @brief Remote inteface of a property:
	 */
	template<typename ZEUS_TYPE_PROPERTY>
	class RemoteProperty {
		private:
			zeus::ObjectRemote& m_remoteObject;
			std::string m_name;
		public:
			/**
			 * @brief generic constructor
			 */
			RemoteProperty(zeus::ObjectRemote& _remoteObject, const std::string& _name) :
			  m_remoteObject(_remoteObject),
			  m_name(_name) {
				
			}
			/**
			 * @brief Get the property value
			 * @return the requested value
			 */
			zeus::Future<ZEUS_TYPE_PROPERTY> get() {
				return m_remoteObject.call(m_name + ".get");
			}
			/**
			 * @brief Set the property value
			 * @param[in] _value The new value to set
			 */
			zeus::Future<void> set(const ZEUS_TYPE_PROPERTY& _value) {
				return m_remoteObject.call(m_name + ".set", _value);
			}
	};
}


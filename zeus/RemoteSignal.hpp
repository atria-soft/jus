/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
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
	template<typename ZEUS_TYPE_SIGNAL>
	class RemoteSignal : public esignal::Signal<ZEUS_TYPE_SIGNAL> {
		private:
			zeus::ObjectRemote& m_remoteObject; //!< Remote ofject interface
			zeus::Future<ZEUS_TYPE_SIGNAL> m_handleSignal;
		public:
			/**
			 * @brief generic constructor
			 */
			RemoteProperty(zeus::ObjectRemote& _remoteObject, const std::string& _name) :
			  esignal::Signal<ZEUS_TYPE_SIGNAL>(&zeus::RemoteSignal<ZEUS_TYPE_SIGNAL>::countConnection, _name),
			  m_remoteObject(_remoteObject),
			  m_name(_name) {
				
			}
		private:
			/**
			 * @brief local function that is called every time someone connect on the local signal or disconnect
			 * @param[in] _numberConnected Nomber of connected users.
			 */
			void countConnection(size_t _numberConnected) {
				if (_numberConnected == 1) {
					m_handleSignal = m_remoteObject.call(getName() + ".connect");
					m_handleSignal.onSignal(&zeus::RemoteSignal<ZEUS_TYPE_SIGNAL>::onSignal);
				} else if (_numberConnected == 0) {
					// stop the connection on the signalling
					m_handleSignal.abort();
				}
			}
			/**
			 * @brief When remote process emit a signal, it will arrive here, and be retansmit in generic esignal methode
			 * @param[in] _value Data receive from remote user.
			 */
			void onSignal(ZEUS_TYPE_SIGNAL _value) {
				emit(_value);
			}
	};
}


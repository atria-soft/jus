/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <eproperty/Value.h>
#include <esignal/Signal.h>
#include <enet/Tcp.h>
#include <thread>
#include <memory>

namespace jus {
	class TcpString : public eproperty::Interface {
		private:
			enet::Tcp m_connection;
			std::thread* m_thread;
			bool m_threadRunning;
			std::chrono::steady_clock::time_point m_lastReceive;
			std::chrono::steady_clock::time_point m_lastSend;
		public:
			using Observer = std::function<void(std::string)>; //!< Define an Observer: function pointer
			Observer m_obsercerElement;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(std::string)) {
				m_obsercerElement = [=](std::string _value){
					(*_class.*_func)(std::move(_value));
				};
			}
		public:
			TcpString();
			TcpString(enet::Tcp _connection);
			virtual ~TcpString();
			void setInterface(enet::Tcp _connection);
			void connect(bool _async = false);
			void disconnect(bool _inThreadStop = false);
			bool isActive() const;
			void setInterfaceName(const std::string& _name);
			int32_t write(const std::string& _data);
			std::string asyncRead();
		private:
			std::string read();
		private:
			void threadCallback();
		public:
			const std::chrono::steady_clock::time_point& getLastTimeReceive() {
				return m_lastReceive;
			}
			const std::chrono::steady_clock::time_point& getLastTimeSend() {
				return m_lastSend;
			}
	};
}


/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <eproperty/Value.h>
#include <esignal/Signal.h>
#include <jus/Buffer.h>
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
			using ObserverRaw = std::function<void(jus::Buffer&)>; //!< Define an Observer: function pointer
			Observer m_observerElement;
			ObserverRaw m_observerRawElement;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(std::string)) {
				m_observerElement = [=](std::string _value){
					(*_class.*_func)(std::move(_value));
				};
			}
			void connectClean() {
				m_observerElement = nullptr;
			}
			template<class CLASS_TYPE>
			void connectRaw(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(jus::Buffer&)) {
				m_observerRawElement = [=](jus::Buffer& _value){
					(*_class.*_func)(_value);
				};
			}
			void connectCleanRaw() {
				m_observerRawElement = nullptr;
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
			int32_t writeBinary(jus::Buffer& _data);
			std::string asyncRead();
		private:
			std::string read();
			jus::Buffer readRaw();
			std::vector<uint8_t> m_buffer;
		private:
			void threadCallback();
		public:
			const std::chrono::steady_clock::time_point& getLastTimeReceive() {
				return m_lastReceive;
			}
			const std::chrono::steady_clock::time_point& getLastTimeSend() {
				return m_lastSend;
			}
		private:
			using ActionAsync = std::function<bool(TcpString* _interface)>;
			std::mutex m_threadAsyncMutex;
			std::thread* m_threadAsync;
			bool m_threadAsyncRunning;
			std::vector<ActionAsync> m_threadAsyncList;
		private:
			void threadAsyncCallback();
		public:
			void addAsync(ActionAsync _elem) {
				std::unique_lock<std::mutex> lock(m_threadAsyncMutex);
				m_threadAsyncList.push_back(_elem);
			}
	};
}


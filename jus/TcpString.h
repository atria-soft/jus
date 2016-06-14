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
#include <jus/connectionMode.h>
#include <jus/AbstractFunction.h>

namespace jus {
	class TcpString : public eproperty::Interface {
		private:
			enet::Tcp m_connection;
			std::thread* m_thread;
			bool m_threadRunning;
		protected:
			enum jus::connectionMode m_interfaceMode;
		public:
			enum jus::connectionMode getMode() {
				return m_interfaceMode;
			}
			void setMode(enum jus::connectionMode _mode) {
				m_interfaceMode = _mode;
			}
			std::vector<uint8_t> m_buffer;
			std::vector<uint8_t> m_temporaryBuffer;
			std::chrono::steady_clock::time_point m_lastReceive;
			std::chrono::steady_clock::time_point m_lastSend;
		public:
			using Observer = std::function<void(jus::Buffer&)>; //!< Define an Observer: function pointer
			Observer m_observerElement;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(jus::Buffer&)) {
				m_observerElement = [=](jus::Buffer& _value){
					(*_class.*_func)(_value);
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
			int32_t writeJson(ejson::Object& _data);
			int32_t writeBinary(jus::Buffer& _data);
			std::string asyncRead();
		private:
			void read();
			jus::Buffer readRaw();
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
			
			
			
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			template<class JUS_ARG>
			void answerValue(uint64_t _clientTransactionId, JUS_ARG _value, uint32_t _clientId=0) {
				if (m_interfaceMode == jus::connectionMode::modeJson) {
					ejson::Object answer;
					answer.add("id", ejson::Number(_clientTransactionId));
					if (_clientId != 0) {
						answer.add("client-id", ejson::Number(_clientId));
					}
					std::vector<jus::ActionAsyncClient> asyncAction;
					answer.add("return", jus::convertToJson(asyncAction, -1, _value));
					if (asyncAction.size() != 0) {
						JUS_ERROR("ASYNC datas ... TODO ///");
					}
					writeJson(answer);
				} else if (m_interfaceMode == jus::connectionMode::modeBinary) {
					jus::Buffer answer;
					answer.setType(jus::Buffer::typeMessage::answer);
					answer.setTransactionId(_clientTransactionId);
					answer.setClientId(_clientId);
					answer.addAnswer(_value);
					writeBinary(answer);
				} else if (m_interfaceMode == jus::connectionMode::modeXml) {
					JUS_ERROR("TODO ... ");
				} else {
					JUS_ERROR("wrong type of communication");
				}
			}
			void answerVoid(uint64_t _clientTransactionId, uint32_t _clientId=0);
			void answerError(uint64_t _clientTransactionId, const std::string& _errorValue, const std::string& _errorComment="", uint32_t _clientId=0);
	};
}


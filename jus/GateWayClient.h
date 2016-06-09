/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <ememory/memory.h>
#include <esignal/Signal.h>
#include <jus/GateWayService.h>
#include <jus/Future.h>
#include <jus/AbstractFunction.h>
#include <jus/connectionMode.h>



namespace jus {
	class GateWay;
	class GateWayClient {
		private:
			enum class state {
				unconnect, // starting sate
				connect, // just get a TCP connection
				userIdentify, // client set the user it want to access
				clientIdentify, // client defien the mode of the acces (anonymous,client/user)
				disconnect // client is dead or loal disconnection
			};
			enum state m_state; // state machine ...
		private:
			jus::GateWay* m_gatewayInterface;
			jus::TcpString m_interfaceClient;
		protected:
			enum jus::connectionMode m_interfaceMode;
		public:
			enum jus::connectionMode getMode() { return m_interfaceMode; }
		public:
			esignal::Signal<bool> signalIsConnected;
			ememory::SharedPtr<jus::GateWayService> m_userService;
			std::vector<ememory::SharedPtr<jus::GateWayService>> m_listConnectedService;
			uint64_t m_uid;
			uint64_t m_uid2;
			std::string m_userConnectionName;
			std::string m_clientName;
			std::vector<std::string> m_clientgroups;
			std::vector<std::string> m_clientServices;
		public:
			GateWayClient(enet::Tcp _connection, jus::GateWay* _gatewayInterface);
			virtual ~GateWayClient();
			void start(uint64_t _uid, uint64_t _uid2);
			void stop();
			void onClientDataRaw(const jus::Buffer& _value);
			void onClientData(std::string _value);
			void returnMessage(ejson::Object _data);
			bool checkId(uint64_t _id) const {
				return    m_uid == _id
				       || m_uid2 == _id;
			}
			bool isAlive();
			
			
		private:
			std::mutex m_mutex;
			std::vector<std::pair<uint64_t, jus::FutureBase>> m_pendingCall;
			int32_t m_transactionLocalId;
			jus::FutureBase callJson(uint64_t _callerId, ememory::SharedPtr<jus::GateWayService> _srv, uint64_t _clientTransactionId, uint64_t _transactionId, const ejson::Object& _obj, jus::FutureData::ObserverFinish _callback=nullptr);
			uint64_t getId();
		public:
			template<class... _ARGS>
			jus::FutureBase call(uint64_t _callerId, ememory::SharedPtr<jus::GateWayService> _srv, const std::string& _functionName, _ARGS&&... _args) {
				uint64_t id = getId();
				std::vector<ActionAsyncClient> asyncAction;
				ejson::Object callElem = jus::createCall(asyncAction, id, _functionName, std::forward<_ARGS>(_args)...);
				if (asyncAction.size() != 0) {
					JUS_ERROR("Missing send async messages");
				}
				return callJson(_callerId, _srv, 0, id, callElem);
			}
			template<class... _ARGS>
			jus::FutureBase callAction(uint64_t _callerId, ememory::SharedPtr<jus::GateWayService> _srv, const std::string& _functionName, _ARGS&&... _args, jus::FutureData::ObserverFinish _callback) {
				uint64_t id = getId();
				std::vector<ActionAsyncClient> asyncAction;
				ejson::Object callElem = jus::createCall(asyncAction, id, _functionName, std::forward<_ARGS>(_args)...);
				if (asyncAction.size() != 0) {
					JUS_ERROR("Missing send async messages");
				}
				return callJson(_callerId, _srv, 0, id, callElem, _callback);
			}
			jus::FutureBase callActionForward(uint64_t _callerId,
			                                  uint64_t _clientTransactionId,
			                                  ememory::SharedPtr<jus::GateWayService> _srv,
			                                  const std::string& _functionName,
			                                  ejson::Array _params,
			                                  jus::FutureData::ObserverFinish _callback,
			                                  int64_t _part,
			                                  bool _finish);
			
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			
			template<class JUS_ARG>
			void answerValue(uint64_t _clientTransactionId, JUS_ARG _value) {
				if (m_interfaceMode == jus::connectionMode::modeJson) {
					ejson::Object answer;
					answer.add("id", ejson::Number(_clientTransactionId));
					std::vector<ActionAsyncClient> asyncAction;
					answer.add("return", jus::convertToJson(asyncAction, -1, _value));
					if (asyncAction.size() != 0) {
						JUS_ERROR("ASYNC datas ... TODO ///");
					}
					JUS_DEBUG("answer: " << answer.generateHumanString());
					m_interfaceClient.write(answer.generateMachineString());
				} else if (m_interfaceMode == jus::connectionMode::modeBinary) {
					jus::Buffer answer;
					answer.setType(jus::Buffer::typeMessage::answer);
					answer.setTransactionId(_clientTransactionId);
					answer.addAnswer(_value);
					JUS_DEBUG("answer: " << answer.generateHumanString());
					m_interfaceClient.writeBinary(answer);
				} else if (m_interfaceMode == jus::connectionMode::modeXml) {
					JUS_ERROR("TODO ... ");
				} else {
					JUS_ERROR("wrong type of communication");
				}
			}
			
			void answerError(uint64_t _clientTransactionId, const std::string& _errorValue, const std::string& _errorComment="");
			
			
	};
}


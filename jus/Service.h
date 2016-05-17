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

namespace jus {
	class Service : public eproperty::Interface {
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
		private:
			jus::TcpString m_interfaceClient;
			uint32_t m_id;
			esignal::Connection m_dataCallback;
			std::vector<std::string> m_newData;
		public:
			Service();
			virtual ~Service();
			// Genenric function call:
			ejson::Object callJson(const ejson::Object& _obj);
			void connect();
			void disconnect();
		private:
			void onClientData(const std::string& _value);
			std::string asyncRead();
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
		protected:
			
			void createSignatureInternal(std::vector<std::string>& _signature) {
				// Finish recursive parse ...
			}
			template<class... _ARGS>
			void createSignatureInternal(std::vector<std::string>& _signature, const std::string& _param, _ARGS&&... _args) {
				_signature.push_back("string");
				createSignatureInternal(_signature, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createSignatureInternal(std::vector<std::string>& _signature, const bool& _param, _ARGS&&... _args) {
				_signature.push_back("bool");
				createSignatureInternal(_signature, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createSignatureInternal(std::vector<std::string>& _signature, const double& _param, _ARGS&&... _args) {
				_signature.push_back("double");
				createSignatureInternal(_signature, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			void createSignatureInternal(std::vector<std::string>& _signature, const int32_t& _param, _ARGS&&... _args) {
				_signature.push_back("int32");
				createSignatureInternal(_signature, std::forward<_ARGS>(_args)...);
			}
			template<class... _ARGS>
			std::vector<std::string> createSignature(_ARGS&&... _args) {
				std::vector<std::string> signature;
				createSignatureInternal(signature, std::forward<_ARGS>(_args)...);
				return signature;
			}
			
			template<class JUS_RETURN_VALUE,
			         class JUS_CLASS_TYPE,
			         class... JUS_FUNC_ARGS_TYPE>
			void advertise(const std::string& _name,
			               JUS_RETURN_VALUE (JUS_CLASS_TYPE::*_func)(const JUS_FUNC_ARGS_TYPE&... _args),
			               const std::string& _desc) {
				/*
				std::vector<std::string> plop = createSignature(_args):
				JUS_ERROR("signature:");
				for (auto& it : plop) {
					JUS_ERROR("    - " << it);
				}
				*/
			}
	};
}


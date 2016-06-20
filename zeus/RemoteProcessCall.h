/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <eproperty/Value.h>
#include <jus/AbstractFunctionTypeDirect.h>
#include <jus/AbstractFunctionTypeClass.h>
#include <jus/debug.h>

namespace jus {
	class RemoteProcessCall {
		public:
			RemoteProcessCall();
		protected:
			std::vector<jus::AbstractFunction*> m_listFunction;
		protected:
			std::string m_description;
		public:
			void setDescription(const std::string& _desc);
			std::string getDescription();
		protected:
			std::string m_version;
		public:
			void setVersion(const std::string& _desc);
			std::string getVersion();
		protected:
			std::vector<std::pair<std::string,std::string>> m_authors;
		public:
			void addAuthor(const std::string& _name, const std::string& _email);
			const std::vector<std::pair<std::string,std::string>>& getAuthors() const;
			std::vector<std::string> getAuthors2();
		protected:
			std::string m_type;
		public:
			std::string getType();
			void setType(const std::string& _type, uint16_t _version);
		public:
			std::vector<std::string> getFunctions();
			std::vector<std::string> getFunctionSignature(std::string _funcName);
			std::string getFunctionPrototype(std::string _funcName);
			std::string getFunctionDescription(std::string _funcName);
		public:
			void setLastFuncDesc(const std::string& _desc);
			void setFuncDesc(const std::string& _funcName, const std::string& _desc);
			void addLastFuncParam(const std::string& _name, const std::string& _desc);
			void setFuncParam(const std::string& _funcName, int32_t _idParam, const std::string& _name, const std::string& _desc);
			void addLastFuncReturn(const std::string& _desc);
			void setFuncReturn(const std::string& _funcName, const std::string& _desc);
		protected:
			virtual bool isFunctionAuthorized(uint64_t _clientSessionID, const std::string& _funcName);
		public:
			// Add global fuction (no link with this class)
			template<class JUS_RETURN_VALUE,
			         class... JUS_FUNC_ARGS_TYPE>
			void advertise(const std::string& _name,
			               JUS_RETURN_VALUE (*_func)(JUS_FUNC_ARGS_TYPE... _args),
			               const std::string& _desc = "") {
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					if (it->getName() == _name) {
						JUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return;
					}
				}
				AbstractFunction* tmp = createAbstractFunctionDirect(_name, _desc, _func);
				if (tmp == nullptr) {
					JUS_ERROR("can not create abstract function ... '" << _name << "'");
					return;
				}
				tmp->setType(jus::AbstractFunction::type::global);
				JUS_INFO("Add function '" << _name << "' in global mode");
				m_listFunction.push_back(tmp);
			}
			// Add Local fuction (depend on this class)
			template<class JUS_RETURN_VALUE,
			         class JUS_CLASS_TYPE,
			         class... JUS_FUNC_ARGS_TYPE>
			void advertise(std::string _name,
			               JUS_RETURN_VALUE (JUS_CLASS_TYPE::*_func)(JUS_FUNC_ARGS_TYPE... _args),
			               const std::string& _desc = "") {
				_name = "sys." + _name;
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					if (it->getName() == _name) {
						JUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return;
					}
				}
				AbstractFunction* tmp = createAbstractFunctionClass(_name, _desc, _func);
				if (tmp == nullptr) {
					JUS_ERROR("can not create abstract function ... '" << _name << "'");
					return;
				}
				tmp->setType(jus::AbstractFunction::type::local);
				JUS_INFO("Add function '" << _name << "' in local mode");
				m_listFunction.push_back(tmp);
			}
	};
}


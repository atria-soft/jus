/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <eproperty/Value.h>
#include <ejson/ejson.h>
#include <jus/AbstractFunctionTypeDirect.h>
#include <jus/AbstractFunctionTypeClass.h>
#include <jus/debug.h>

namespace jus {
	class RemoteProcessCall {
		protected:
			std::vector<jus::AbstractFunction*> m_listFunction;
		protected:
			std::string m_description;
		public:
			void setDescription(const std::string& _desc);
			const std::string& getDescription() const;
		protected:
			std::string m_version;
		public:
			void setVersion(const std::string& _desc);
			const std::string& getVersion() const;
		protected:
			std::vector<std::pair<std::string,std::string>> m_authors;
		public:
			void addAuthor(const std::string& _name, const std::string& _email);
			const std::vector<std::pair<std::string,std::string>>& getAuthors() const;
		public:
			void setLastFuncDesc(const std::string& _desc);
			void setFuncDesc(const std::string& _funcName, const std::string& _desc);
			void addLastFuncParam(const std::string& _name, const std::string& _desc);
			void setFuncParam(const std::string& _funcName, int32_t _idParam, const std::string& _name, const std::string& _desc);
		public:
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
				m_listFunction.push_back(tmp);
			}
	};
}


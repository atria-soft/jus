/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.h>
#include <zeus/AbstractFunctionTypeDirect.h>
#include <zeus/AbstractFunctionTypeClass.h>
#include <zeus/debug.h>

namespace zeus {
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	class RemoteProcessCall {
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			RemoteProcessCall();
		protected:
			std::vector<zeus::AbstractFunction*> m_listFunction;
		protected:
			std::string m_description;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void setDescription(const std::string& _desc);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			std::string getDescription();
		protected:
			std::string m_version;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void setVersion(const std::string& _desc);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			std::string getVersion();
		protected:
			std::vector<std::pair<std::string,std::string>> m_authors;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void addAuthor(const std::string& _name, const std::string& _email);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			const std::vector<std::pair<std::string,std::string>>& getAuthors() const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			std::vector<std::string> getAuthors2();
		protected:
			std::string m_type;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			std::string getType();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			void setType(const std::string& _type, uint16_t _version);
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			std::vector<std::string> getFunctions();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			AbstractFunction* getFunction(std::string _funcName);
		private:
			std::vector<std::string> getFunctionSignature(std::string _funcName);
			std::string getFunctionPrototype(std::string _funcName);
			std::string getFunctionDescription(std::string _funcName);
		protected:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			virtual bool isFunctionAuthorized(uint64_t _clientSessionID, const std::string& _funcName);
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			// Add global fuction (no link with this class)
			template<class ZEUS_RETURN_VALUE,
			         class... ZEUS_FUNC_ARGS_TYPE>
			zeus::AbstractFunction* advertise(const std::string& _name,
			                                  ZEUS_RETURN_VALUE (*_func)(ZEUS_FUNC_ARGS_TYPE... _args)) {
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					if (it->getName() == _name) {
						ZEUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return nullptr;
					}
				}
				zeus::AbstractFunction* tmp = createAbstractFunctionDirect(_name, _func);
				if (tmp == nullptr) {
					ZEUS_ERROR("can not create abstract function ... '" << _name << "'");
					return nullptr;
				}
				tmp->setType(zeus::AbstractFunction::type::global);
				ZEUS_INFO("Add function '" << _name << "' in global mode");
				m_listFunction.push_back(tmp);
				return tmp;
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			// Add Local fuction (depend on this class)
			template<class ZEUS_RETURN_VALUE,
			         class ZEUS_CLASS_TYPE,
			         class... ZEUS_FUNC_ARGS_TYPE>
			zeus::AbstractFunction* advertise(std::string _name,
			                                  ZEUS_RETURN_VALUE (ZEUS_CLASS_TYPE::*_func)(ZEUS_FUNC_ARGS_TYPE... _args)) {
				_name = "sys." + _name;
				for (auto &it : m_listFunction) {
					if (it == nullptr) {
						continue;
					}
					if (it->getName() == _name) {
						ZEUS_ERROR("Advertise function already bind .. ==> can not be done...: '" << _name << "'");
						return nullptr;
					}
				}
				zeus::AbstractFunction* tmp = createAbstractFunctionClass(_name, _func);
				if (tmp == nullptr) {
					ZEUS_ERROR("can not create abstract function ... '" << _name << "'");
					return nullptr;
				}
				tmp->setType(zeus::AbstractFunction::type::local);
				ZEUS_INFO("Add function '" << _name << "' in local mode");
				m_listFunction.push_back(tmp);
				return tmp;
			}
	};
}


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
	 * @brief Local declaration of call local data
	 */
	class RemoteProcessCall : public zeus::WebObj {
		public:
			/**
			 * @brief Basic constructor
			 * @param[in] _iface web interface to interact network
			 * @param[in] _id Id of the service interface
			 * @param[in] _objectId Local object interface
			 */
			RemoteProcessCall(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _id, uint16_t _objectId);
		protected:
			std::vector<zeus::AbstractFunction*> m_listFunction; //!< List of all functions callable
		protected:
			std::string m_description; //!< Description of the service
		public:
			/**
			 * @brief Set service description
			 * @param[in] _desc String with the describe of the service
			 */
			void setDescription(const std::string& _desc);
			/**
			 * @brief Get service description
			 * @return String with the describe of the service
			 */
			std::string getDescription();
		protected:
			std::string m_version; //!< Version of the service
		public:
			/**
			 * @brief Set the Version of the service
			 * @param[in] _vers String containing the version (form: 1.0[.x[.y]][-dev]
			 */
			void setVersion(const std::string& _vers);
			/**
			 * @brief Get the Version of the service
			 * @return String containing the version (form: 1.0[.x[.y]][-dev]
			 */
			std::string getVersion();
		protected:
			std::string m_versionImplement; //!< Version implementation of the service
		public:
			/**
			 * @brief Set the Version implementation of the service
			 * @param[in] _vers String containing the version (form: 1.0[.x[.y]][-dev]
			 */
			void setVersionImplementation(const std::string& _vers);
			/**
			 * @brief Get the Version implementation of the service
			 * @return String containing the version (form: 1.0[.x[.y]][-dev]
			 */
			std::string getVersionImplementation();
		protected:
			std::vector<std::pair<std::string,std::string>> m_authors;//! List of autors of the module (name, email)
		public:
			/**
			 * @brief Add an author on this service
			 * @param[in] _name Nazme of the Author: (Surname NAME)
			 * @param[in] _email email of the author to add
			 */
			void addAuthor(const std::string& _name, const std::string& _email);
			/**
			 * @brief Get the list of the Authors
			 * @return Lisl of authors in a pair of name and email
			 */
			const std::vector<std::pair<std::string,std::string>>& getAuthors() const;
			/**
			 * @brief Get simple list of authors
			 * @return List Of user and email in form: "john WHO <jhon.who@here.net>"
			 */
			std::vector<std::string> getAuthors2();
		protected:
			std::string m_type; //!< Generic type of the service
		public:
			/**
			 * @brief Get type of this object
			 * @return Type of this object in string
			 */
			std::string getType();
			/**
			 * @brief Set the type of this object
			 * @param[in] _type New type of this object
			 */
			void setType(const std::string& _type);
		public:
			/**
			 * @brief Get list of fucntion aaillable in this object
			 * @return List of function name
			 */
			std::vector<std::string> getFunctions();
			/**
			 * @brief Get the AbstractFunction of a function with a specific name
			 * @param[in] _funcName Name of the function
			 * @return Pointer on the function
			 */
			AbstractFunction* getFunction(std::string _funcName);
		private:
			/**
			 * @brief Get the signature of a function with a specific name
			 * @param[in] _funcName Name of the function
			 * @return IDL function signature
			 */
			std::vector<std::string> getFunctionSignature(std::string _funcName);
			/**
			 * @brief Get the signature of a function with a specific name
			 * @param[in] _funcName Name of the function
			 * @return IDL function prototype
			 */
			std::string getFunctionPrototype(std::string _funcName);
			/**
			 * @brief Get the descriptyion of a function with a specific name
			 * @param[in] _funcName Name of the function
			 * @return Description of this function
			 */
			std::string getFunctionDescription(std::string _funcName);
		protected:
			/**
			 * @brief Check if the function is accessible for a specific user
			 * @param[in] _clientId User unique ID
			 * @param[in] _funcName Function name that the user is calling
			 * @return true The function is authorized
			 * @return false The function is NOT authorized
			 */
			virtual bool isFunctionAuthorized(uint64_t _clientSessionID, const std::string& _funcName);
		public:
			/**
			 * @brief Advertise a new function in the service/object.
			 * @param[in] _name Name of the function
			 * @param[in] _func pointer on the function that might be used to call it.
			 * @return an handle on an abstract function that can be called.
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
				ZEUS_VERBOSE("Add function '" << _name << "' in global mode");
				m_listFunction.push_back(tmp);
				return tmp;
			}
			/**
			 * @brief Advertise a new function in the service/object.
			 * @param[in] _name Name of the function
			 * @param[in] _func pointer on the function that might be used to call it.
			 * @return an handle on an abstract function that can be called.
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
				ZEUS_VERBOSE("Add function '" << _name << "' in local mode");
				m_listFunction.push_back(tmp);
				return tmp;
			}
	};
}


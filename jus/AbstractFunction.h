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
#include <jus/ParamType.h>

class AbstractFunction {
	protected:
		const std::string m_name;
	public:
		const std::string& getName() const {
			return m_name;
		}
	protected:
		const std::string m_description;
	public:
		const std::string& getDescription() const {
			return m_description;
		}
	protected:
		AbstractFunction(const std::string& _name,
		        const std::string& _desc):
		  m_name(_name),
		  m_description(_desc) {
			
		}
	public:
		virtual ~AbstractFunction() {};
		bool checkCompatibility(const ParamType& _type, const ejson::Value& _params);
		bool checkCompatibility(const ParamType& _type, const std::string& _params);
	public:
		virtual std::string getPrototype() const = 0;
		virtual ejson::Value executeJson(const ejson::Array& _params, void* _class=nullptr) = 0;
		virtual std::string executeString(const std::vector<std::string>& _params, void* _class=nullptr) = 0;
};

template<class JUS_TYPE>
JUS_TYPE convertStringTo(const std::string& _value);

template<class JUS_TYPE>
JUS_TYPE convertJsonTo(const ejson::Value& _value);

template<class JUS_TYPE>
ejson::Value convertToJson(const JUS_TYPE& _value);
/*
template <class, class...>
class TypeList;
*/


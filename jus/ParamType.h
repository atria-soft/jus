/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

class ParamType {
	protected:
		const char* m_typeName;
	public:
		ParamType(const char* _name = ""):
		  m_typeName(_name) {
			
		}
		const char* getName() const {
			return m_typeName;
		}
		bool operator == (const ParamType& _obj) const {
			return m_typeName == _obj.m_typeName;
		}
};


template<class JUS_TYPE>
ParamType createType();



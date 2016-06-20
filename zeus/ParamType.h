/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <string>

namespace zeus {
	class ParamType {
		protected:
			const char* m_typeName;
		public:
			ParamType(const char* _name = "");
			const char* getName() const;
			bool operator == (const ParamType& _obj) const;
			bool operator == (const std::string& _value) const;
			bool operator != (const std::string& _value) const;
	};
	
	
	template<class ZEUS_TYPE>
	ParamType createType();
}


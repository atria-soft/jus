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
			const std::string m_typeName;
			const uint16_t m_id;
			const bool m_isNumber;
			const bool m_isVector;
		public:
			ParamType(const char* _name = "", uint16_t _id=0, bool _isNumber=false, bool _isVector=false);
			ParamType(const std::string& _name, uint16_t _id, bool _isNumber=false, bool _isVector=false);
			const std::string& getName() const;
			uint16_t getId() const;
			bool operator == (const ParamType& _obj) const;
			bool operator != (const ParamType& _obj) const;
			bool operator == (const std::string& _value) const;
			bool operator != (const std::string& _value) const;
			bool operator == (const uint16_t& _value) const;
			bool operator != (const uint16_t& _value) const;
			bool isNumber() const;
			bool isVector() const;
	};
	extern const uint16_t paramTypeObject;
	std::ostream& operator <<(std::ostream& _os, const zeus::ParamType& _obj);
	
	template<class ZEUS_TYPE>
	ParamType createType();
}


/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <string>

namespace zeus {
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	class ParamType {
		protected:
			const std::string m_typeName;
			const uint16_t m_id;
			const bool m_isNumber;
			const bool m_isVector;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			ParamType(const char* _name = "", uint16_t _id=0, bool _isNumber=false, bool _isVector=false);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			ParamType(const std::string& _name, uint16_t _id, bool _isNumber=false, bool _isVector=false);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			const std::string& getName() const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			uint16_t getId() const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool operator == (const ParamType& _obj) const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool operator != (const ParamType& _obj) const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool operator == (const std::string& _value) const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool operator != (const std::string& _value) const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool operator == (const uint16_t& _value) const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool operator != (const uint16_t& _value) const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool isNumber() const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool isVector() const;
	};
	extern const uint16_t paramTypeObject;
	extern const uint16_t paramTypeRaw;
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	std::ostream& operator <<(std::ostream& _os, const zeus::ParamType& _obj);
	
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	template<class ZEUS_TYPE>
	ParamType createType();
	/**
	 * @brief Check the compatibility of 2 parameter type
	 * @param[in] _first First parameter to check
	 * @param[in] _second Second parameter to check
	 * @return Return true if parameters are compatible (thay can be converted)
	 */
	bool checkCompatibility(const ParamType& _first, const ParamType& _second);
}


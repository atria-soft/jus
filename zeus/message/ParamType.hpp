/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <string>

namespace zeus {
	namespace message {
		/**
		 * @brief generisation of type of the type of the parameter
		 */
		class ParamType {
			protected:
				const std::string m_typeName; //!< generic type
				const uint16_t m_id; //!< simplification ID (if possible)
				const bool m_isNumber; //!< if the element is a number (convertion possible)
				const bool m_isVector; //!< if the element is a vector (convertion possible)
			public:
				/**
				 * @brief contructor onf a generic name parameter
				 * @param[in] _name Name of the parameter
				 * @param[in] _id Generic Id of the parameter
				 * @param[in] _isNumber set true of the type is a number
				 * @param[in] _isVector set true of the type is a vector element
				 * @return 
				 */
				ParamType(const char* _name = "", uint16_t _id=0, bool _isNumber=false, bool _isVector=false);
				/**
				 * @copydoc zeus::message::ParamType::ParamType
				 */
				ParamType(const std::string& _name, uint16_t _id, bool _isNumber=false, bool _isVector=false);
				/**
				 * @brief Get name of tha parameter
				 * @return string describing the TYPE
				 */
				const std::string& getName() const;
				/**
				 * @brief Get generic Id of the type
				 * @return unsigned int containing the type
				 */
				uint16_t getId() const;
				/**
				 * @brief Egality comparaison with an other parameter
				 * @param[in] _obj Other parameter to compare type
				 * @return true if the 2 object are identical
				 */
				bool operator == (const ParamType& _obj) const;
				/**
				 * @brief Difference comparaison with an other parameter
				 * @param[in] _obj Other parameter to compare type
				 * @return true if the 2 object are different
				 */
				bool operator != (const ParamType& _obj) const;
				/**
				 * @brief Egality comparaison with an other parameter
				 * @param[in] _obj Other parameter to compare type in strin
				 * @return true if the 2 object are identical
				 */
				bool operator == (const std::string& _value) const;
				/**
				 * @brief Difference comparaison with an other parameter
				 * @param[in] _obj Other parameter to compare type in string
				 * @return true if the 2 object are different
				 */
				bool operator != (const std::string& _value) const;
				/**
				 * @brief Egality comparaison with an other parameter
				 * @param[in] _obj Other parameter to compare type enum integer
				 * @return true if the 2 object are identical
				 */
				bool operator == (const uint16_t& _value) const;
				/**
				 * @brief Difference comparaison with an other parameter
				 * @param[in] _obj Other parameter to compare type enum integer
				 * @return true if the 2 object are different
				 */
				bool operator != (const uint16_t& _value) const;
				/**
				 * @brief Check if the type is a number
				 * @return return true if the type is a number
				 */
				bool isNumber() const;
				/**
				 * @brief Check if the type is a vector
				 * @return return true if the type is a vector
				 */
				bool isVector() const;
		};
		extern const uint16_t paramTypeObject; //!< van not automatic create a type with the string named object
		/**
		 * @brief Create human readable stream to debug
		 * @param[in] _os the stream to inject data
		 * @param[in] _obj Object to display
		 * @return The inpout stream
		 */
		std::ostream& operator <<(std::ostream& _os, const zeus::message::ParamType& _obj);
		/**
		 * @brief Template to automaticly get the type of an generic std type without create a dynamic element
		 * @return generic parameter created
		 */
		template<class ZEUS_TYPE>
		const ParamType& createType();
		/**
		 * @brief Check the compatibility of 2 parameter type
		 * @param[in] _first First parameter to check
		 * @param[in] _second Second parameter to check
		 * @return Return true if parameters are compatible (thay can be converted)
		 */
		bool checkCompatibility(const ParamType& _first, const ParamType& _second);
	}
}


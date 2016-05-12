/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once


namespace jus {
	class Service {
		public:
			//eproperty::Value<std::string> propertyServiceName;
		public:
			Service();
			virtual ~Service();
		private:
			// TODO: ...
			void bind(const std::string& _functionName) {}
	};
}


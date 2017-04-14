/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/Media.hpp>
#include <etk/os/FSNode.hpp>
#include <echrono/Time.hpp>
#include <ejson/ejson.hpp>

namespace zeus {
	class MediaImpl : public zeus::Media {
		private:
			uint64_t m_id; //!< use local reference ID to have faster access on the file ...
			std::string m_basePath; //!< basic global path
			std::string m_fileName; // Sha 512
			std::string m_name;
			std::string m_mineType;
			echrono::Time m_creationData;
			std::map<std::string, std::string> m_metadata; //!< all extra property
		public:
			/**
			 * @brief Generic json constructor
			 */
			MediaImpl(const std::string& _basePath, ejson::Object _property);
			/**
			 * @brief Generic file constructor
			 */
			MediaImpl(uint64_t _id, const std::string& _fileNameReal);
			/**
			 * @brief Generic destructor
			 */
			~MediaImpl();
			uint64_t getUniqueId() override;
			std::string getMineType() override;
			ememory::SharedPtr<zeus::File> getFile() override;
			std::vector<std::string> getMetadataKeys() override;
			std::string getMetadata(std::string _key) override;
			void setMetadata(std::string _key, std::string _value) override;
			/**
			 * @brief Export json property
			 * @return A json Object with data 
			 */
			ejson::Object getJson();
			const std::string& getFileName() {
				return m_fileName;
			}
			bool erase();
		
	};
}

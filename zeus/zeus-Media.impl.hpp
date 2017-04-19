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
			std::string m_fileName; //!< Name of the file
			std::map<std::string, std::string> m_metadata; //!< all extra property
			std::function<void(zeus::MediaImpl*, const std::string& )> m_callback;
		public:
			void setCallbackMetadataChange(std::function<void(zeus::MediaImpl*, const std::string& )> _callback) {
				m_callback = _callback;
			}
		public:
			/**
			 * @brief Generic json constructor
			 */
			MediaImpl(const std::string& _basePath, ejson::Object _property);
			/**
			 * @brief Generic file constructor
			 */
			MediaImpl(uint64_t _id, const std::string& _fileNameReal, const std::string& _basePath="");
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
			std::string getSha512() override;
			std::string getDecoratedName() override;
			/**
			 * @brief Export json property
			 * @return A json Object with data 
			 */
			ejson::Object getJson();
			const std::string& getFileName() {
				return m_fileName;
			}
			bool erase();
			bool move(const std::string& _newOffsetFile);
			const std::map<std::string, std::string>& getMetadataDirect() {
				return m_metadata;
			}
			void forceUpdateDecoratedName();
		public:
			// local tools:
			static std::string getMetadataFrom(const std::map<std::string, std::string>& _metadata, std::string _key);
			static std::string getDecoratedNameFrom(const std::map<std::string, std::string>& _metadata);
		
	};
}

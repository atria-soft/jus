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
			etk::String m_basePath; //!< basic global path
			etk::String m_fileName; //!< Name of the file
			etk::Map<etk::String, etk::String> m_metadata; //!< all extra property
			std::function<void(zeus::MediaImpl*, const etk::String& )> m_callback;
		public:
			void setCallbackMetadataChange(std::function<void(zeus::MediaImpl*, const etk::String& )> _callback) {
				m_callback = _callback;
			}
		public:
			/**
			 * @brief Generic json constructor
			 */
			MediaImpl(const etk::String& _basePath, ejson::Object _property);
			/**
			 * @brief Generic file constructor
			 */
			MediaImpl(uint64_t _id, const etk::String& _fileNameReal, const etk::String& _basePath="");
			/**
			 * @brief Generic destructor
			 */
			~MediaImpl();
			uint64_t getUniqueId() override;
			etk::String getMineType() override;
			ememory::SharedPtr<zeus::File> getFile() override;
			etk::Vector<etk::String> getMetadataKeys() override;
			etk::String getMetadata(etk::String _key) override;
			void setMetadata(etk::String _key, etk::String _value) override;
			etk::String getSha512() override;
			etk::String getDecoratedName() override;
			/**
			 * @brief Export json property
			 * @return A json Object with data 
			 */
			ejson::Object getJson();
			const etk::String& getFileName() {
				return m_fileName;
			}
			bool erase();
			bool move(const etk::String& _newOffsetFile);
			const etk::Map<etk::String, etk::String>& getMetadataDirect() {
				return m_metadata;
			}
			void forceUpdateDecoratedName();
		public:
			// local tools:
			static etk::String getMetadataFrom(const etk::Map<etk::String, etk::String>& _metadata, etk::String _key);
			static etk::String getDecoratedNameFrom(const etk::Map<etk::String, etk::String>& _metadata);
		
	};
}

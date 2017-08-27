/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#pragma once

#include <ewol/widget/WidgetScrolled.hpp>
#include <ewol/compositing/Image.hpp>
#include <ewol/compositing/Text.hpp>
#include <ewol/compositing/Drawing.hpp>
#include <ewol/widget/Manager.hpp>
#include <esignal/Signal.hpp>
#include <zeus/service/ProxyVideo.hpp>

#include <zeus/Client.hpp>
#include <appl/ClientProperty.hpp>

namespace appl {
	enum class statusLoadingData {
		noData,
		inProgress,
		done
	};
	class ElementProperty : public ememory::EnableSharedFromThis<appl::ElementProperty> {
		private:
			zeus::service::ProxyVideo m_remoteServiceVideo;
			ewol::WidgetShared m_widget;
		public:
			ElementProperty(zeus::service::ProxyVideo& _remoteServiceVideo, ewol::WidgetShared _widget):
			  m_remoteServiceVideo(_remoteServiceVideo),
			  m_widget(_widget),
			  m_metadataUpdated(appl::statusLoadingData::noData),
			  m_nbElementLoaded(0),
			  m_thumbPresent(false) {
				
			}
		public:
			enum appl::statusLoadingData m_metadataUpdated; //!< Check value to know when metadata is getted (like thumb ...)
			uint32_t m_nbElementLoaded; //!< this cont the number of lement loaded to set tle media full loaded
		public:
			bool LoadDataEnded();
			std::mutex m_mutex;
			uint64_t m_id; //!< Remote Id of the Media
			
			egami::Image m_thumb; //!< simple image describing the element
			bool m_thumbPresent; //!< if true, the Image is loaded
			etk::String m_title; //!< Title of the Element
			etk::String m_description; //!< Description of the element
			etk::String m_serie; //!< Name of the serie or empty
			etk::String m_episode; //!< Id of the Episode or empty
			etk::String m_saison; //!< id of the saison or empty
			etk::String m_mineType; //!< mine type: video/audio/image/...
			etk::String m_type; //!< type of the element
			etk::String m_productMethode; //!< Methode of production
			// TODO: float m_globalNote; //!< note over [0,0..1,0]
			// TODO: int32_t m_countPersonalView; //!< number of view this media
			// TODO: int64_t m_globalPersonalView; //!< number of time this media has been viewed
			void loadData();
	};
	class ElementPropertyGroup : public ememory::EnableSharedFromThis<appl::ElementPropertyGroup> {
		private:
			zeus::service::ProxyVideo m_remoteServiceVideo;
			ewol::WidgetShared m_widget;
			enum appl::statusLoadingData m_metadataUpdated; //!< Check value to know when metadata is getted (like thumb ...)
			uint32_t m_nbElementLoaded; //!< this cont the number of lement loaded to set tle media full loaded
		public:
			ElementPropertyGroup(zeus::service::ProxyVideo& _remoteServiceVideo, ewol::WidgetShared _widget):
			  m_remoteServiceVideo(_remoteServiceVideo),
			  m_widget(_widget),
			  m_metadataUpdated(appl::statusLoadingData::noData),
			  m_nbElementLoaded(0),
			  m_thumbPresent(false) {
				
			}
		public:
			bool LoadDataEnded();
		public:
			std::mutex m_mutex;
			uint64_t m_id; //!< Remote Id of the Media
			etk::String m_title; //!< Title of the Group
			etk::String m_filter; //!< element to add in the filter
			egami::Image m_thumb; //!< simple image describing the element
			bool m_thumbPresent; //!< if true, the Image is loaded
			void loadData();
	};
	class ElementDisplayed {
		public:
			ememory::SharedPtr<appl::ElementProperty> m_property;
			ememory::SharedPtr<appl::ElementPropertyGroup> m_propertyGroup;
			int32_t m_idCurentElement;
			etk::Color<float> m_bgColor;
			vec2 m_pos;
			vec2 m_size;
		protected:
			ewol::compositing::Image m_image;
			ewol::compositing::Text m_text;
			ewol::compositing::Drawing m_draw;
		public:
			void draw() {
				m_draw.draw();
				m_image.draw();
				m_text.draw();
			}
			void generateDisplay(vec2 _startPos, vec2 _size);
			void clear() {
				m_image.clear();
				m_text.clear();
				m_draw.clear();
			}
	};
	
	class ElementCache {
		private:
			etk::String m_name;
			ememory::SharedPtr<ewol::resource::Texture> m_resource;
			echrono::Steady m_lastRequestTime;
	};
	
	class CacheImageManager {
		private:
			etk::Vector<ElementCache> m_imageCoverGroupCache; //!< All image cash of the group cover
			
	};
	
	namespace widget {
		class ListViewer;
		using ListViewerShared = ememory::SharedPtr<appl::widget::ListViewer>;
		using ListViewerWeak = ememory::WeakPtr<appl::widget::ListViewer>;
		class ListViewer : public ewol::widget::WidgetScrolled {
			protected:
				ewol::compositing::Text m_text;
			public:
				esignal::Signal<uint32_t> signalSelect; //!< when select a media to view
				
			protected:
				ememory::SharedPtr<ClientProperty> m_clientProp; //!< Generic entrypoint on the Client
			protected:
				etk::Vector<ememory::SharedPtr<ElementProperty>> m_listElement; //!< list of all element getted in the remote access
				etk::Vector<ememory::SharedPtr<ElementPropertyGroup>> m_listElementGroup; //!< list of all element getted in the remote access
				etk::String m_currentFilter;
				etk::String m_currentGroup;
				etk::Vector<ememory::SharedPtr<ElementDisplayed>> m_listDisplay; //!< list of element in the current local display
				uint64_t m_currentPayed;
			protected:
				//! @brief constructor
				ListViewer();
				void init() override;
			public:
				DECLARE_WIDGET_FACTORY(ListViewer, "ListViewer");
				//! @brief destructor
				virtual ~ListViewer();
			public:
				void onDraw() override;
				void onRegenerateDisplay() override;
			public:
				void setClientProperty(ememory::SharedPtr<ClientProperty> _prop) {
					m_clientProp = _prop;
				}
				void searchElements(etk::String _filter="");
				void searchElementsInternal(const etk::String& _filter, const etk::String& _group="", bool _storeHistory=true);
				bool onEventInput(const ewol::event::Input& _event) override;
			protected:
				etk::Vector<etk::Pair<etk::String,etk::String>> m_history;
			public:
				void backHistory();
			public:
				/**
				 * @brief Generate the event with the previous file property
				 * @return true We find an element
				 * @return false We find nothing
				 */
				bool previous();
				/**
				 * @brief Generate the event with the next file property
				 * @return true We find an element
				 * @return false We find nothing
				 */
				bool next();
		};
	}
}





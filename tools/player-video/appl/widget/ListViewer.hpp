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

#include <zeus/Client.hpp>
#include <appl/ClientProperty.hpp>

namespace appl {
	class ElementProperty {
		public:
			std::mutex m_mutex;
			uint64_t m_id; //!< Remote Id of the Media
			bool m_metadataUpdated; //!< Check value to know when metadata is getted (like thumb ...)
			egami::Image m_thumb; //!< simple image describing the element
			std::string m_title; //!< Title of the Element
			std::string m_description; //!< Description of the element
			std::string m_serie; //!< Name of the serie or empty
			std::string m_episode; //!< Id of the Episode or empty
			std::string m_saison; //!< id of the saison or empty
			std::string m_mineType; //!< mine type: video/audio/image/...
			std::string m_type; //!< type of the element
			std::string m_productMethode; //!< Methode of production
			// TODO: float m_globalNote; //!< note over [0,0..1,0]
			// TODO: int32_t m_countPersonalView; //!< number of view this media
			// TODO: int64_t m_globalPersonalView; //!< number of time this media has been viewed
	};
	class ElementPropertyGroup {
		public:
			std::mutex m_mutex;
			uint64_t m_id; //!< Remote Id of the Media
			std::string m_title; //!< Title of the Group
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
				std::vector<ememory::SharedPtr<ElementProperty>> m_listElement; //!< list of all element getted in the remote access
				std::vector<ememory::SharedPtr<ElementPropertyGroup>> m_listElementGroup; //!< list of all element getted in the remote access
				std::string m_currentFilter;
				std::string m_currentGroup;
				std::vector<ememory::SharedPtr<ElementDisplayed>> m_listDisplay; //!< list of element in the current local display
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
				void searchElements(std::string _filter="");
				void searchElementsInternal(const std::string& _filter, const std::string& _group="");
				bool onEventInput(const ewol::event::Input& _event) override;
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





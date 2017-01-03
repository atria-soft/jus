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

namespace appl {
	class ClientProperty {
		public:
			std::string fromUser;
			std::string toUser;
			std::string pass;
			std::string address;
			uint16_t port;
	};
	class ElementProperty {
		public:
			uint32_t m_id; //!< Remote Id of the Media
			bool m_metadataUpdated; //!< Check value to know when metadata is getted (like thumb ...)
			egami::Image m_thumb; //!< simple image describing the element
			std::string m_title; //!< Title of the Element
			std::string m_description; //!< Description of the element
			std::string m_serie; //!< Name of the serie or empty
			std::string m_episode; //!< Id of the Episode or empty
			std::string m_saison; //!< id of the saison or empty
			std::string m_mineType; //!< mine type: video/audio/image/...
			// TODO: float m_globalNote; //!< note over [0,0..1,0]
			// TODO: int32_t m_countPersonalView; //!< number of view this media
			// TODO: int64_t m_globalPersonalView; //!< number of time this media has been viewed
	};
	class ElementDisplayed {
		public:
			ememory::SharedPtr<appl::ElementProperty> m_property;
			int32_t m_idCurentElement;
			etk::Color<float> m_bgColor;
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
	};
	
	namespace widget {
		class ListViewer;
		using ListViewerShared = ememory::SharedPtr<appl::widget::ListViewer>;
		using ListViewerWeak = ememory::WeakPtr<appl::widget::ListViewer>;
		
		class ListViewer : public ewol::widget::WidgetScrolled {
			protected:
				ewol::compositing::Text m_text;
				
			protected:
				ememory::SharedPtr<ClientProperty> m_clientProp; //!< Generic entrypoint on the Client
			protected:
				std::vector<ememory::SharedPtr<ElementProperty>> m_listElement; //!< list of all element getted in the remote access
				std::vector<ememory::SharedPtr<ElementDisplayed>> m_listDisplay; //!< list of element in the current local display
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
				void searchElements(std::string _filter);
		};
	}
}





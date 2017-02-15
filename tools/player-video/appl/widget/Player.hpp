/** @file
 * @author Edouard DUPIN
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/types.hpp>
#include <ewol/debug.hpp>
#include <ewol/widget/Composer.hpp>
#include <esignal/Signal.hpp>
#include <appl/ClientProperty.hpp>

namespace appl {
	namespace widget {
		class Player;
		using PlayerShared = ememory::SharedPtr<appl::widget::Player>;
		using PlayerWeak = ememory::WeakPtr<appl::widget::Player>;
		class Player : public ewol::widget::Composer {
			public: // properties
				
			public: // signals
				
			protected:
				Player();
				void init() override;
			public:
				DECLARE_WIDGET_FACTORY(Player, "Player");
				virtual ~Player();
			public:
				void onGetFocus() override;
			private:
				// callback functions:
				void onCallbackButtonPrevious();
				void onCallbackButtonNext();
				void onCallbackButtonPlay(const bool& _value);
		};
	};
};

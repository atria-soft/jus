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
		class Connection;
		using ConnectionShared = ememory::SharedPtr<appl::widget::Connection>;
		using ConnectionWeak = ememory::WeakPtr<appl::widget::Connection>;
		/**
		 *  @brief File Chooser is a simple selector of file for opening, saving, and what you want ...
		 *  
		 *  As all other pop-up methode ( wost case we can have) the creating is simple , but event back is not all the time simple:
		 *  
		 *  Fist global static declaration and inclusion:
		 *  [code style=c++]
		 *  #include <ewol/widget/meta/Connection.h>
		 *  [/code]
		 *  
		 *  The first step is to create the file chooser pop-up : (never in the constructor!!!)
		 *  [code style=c++]
		 *  ewol::widget::ConnectionShared tmpWidget = ewol::Widget::Connection::create();
		 *  if (tmpWidget == nullptr) {
		 *  	APPL_ERROR("Can not open File chooser !!! ");
		 *  	return -1;
		 *  }
		 *  // register on the Validate event:
		 *  tmpWidget->signalValidate.connect(sharedFromThis(), &****::onCallbackOpenFile);
		 *  // no need of this event watching ...
		 *  tmpWidget->signalCancel.connect(sharedFromThis(), &****::onCallbackClosePopUp);
		 *  // set the title:
		 *   tmpWidget->propertyLabelTitle.set("Open files ...");
		 *  // Set the validate Label:
		 *  tmpWidget->propertyLabelValidate.set("Open");
		 *  // simply set a folder (by default this is the home folder)
		 *  //tmpWidget->propertyPath.set("/home/me");
		 *  // add the widget as windows pop-up ...
		 *  ewol::widget::WindowsShared tmpWindows = getWindows();
		 *  if (tmpWindows == nullptr) {
		 *  	APPL_ERROR("Can not get the current windows !!! ");
		 *  	return -1;
		 *  }
		 *  tmpWindows->popUpWidgetPush(tmpWidget);
		 *  [/code]
		 *  
		 *  Now we just need to wait the the open event message.
		 *  
		 *  [code style=c++]
		 *  void ****::onCallbackOpenFile(const std::string& _value) {
		 *  	APPL_INFO("Request open file : '" << _value << "'");
		 *  }
		 *  void ****::onCallbackClosePopUp() {
		 *  	APPL_INFO("The File chooser has been closed");
		 *  }
		 *  [/code]
		 *  This is the best example of a Meta-widget.
		 */
		class Connection : public ewol::widget::Composer {
			public: // signals
				esignal::Signal<> signalCancel; //!< abort the display of the pop-up or press cancel button
				esignal::Signal<ememory::SharedPtr<appl::ClientProperty>> signalValidate; //!< select file(s)
			protected:
				ememory::SharedPtr<appl::ClientProperty> m_baseProperty;
				Connection();
				void init(ememory::SharedPtr<appl::ClientProperty> _baseProperty=nullptr) override;
			public:
				DECLARE_WIDGET_FACTORY(Connection, "Connection");
				virtual ~Connection();
			private:
				std::string getCompleateFileName();
				void updateCurrentFolder();
			public:
				void onGetFocus() override;
			private:
				// callback functions:
				void onCallbackEntryFolderChangeValue(const std::string& _value);
				void onCallbackEntryFileChangeValue(const std::string& _value);
				void onCallbackButtonCancelPressed();
				void onCallbackHidenFileChangeChangeValue(const bool& _value);
				void onCallbackListFolderSelectChange(const std::string& _value);
				void onCallbackListFileSelectChange(const std::string& _value);
				void onCallbackListFileValidate(const std::string& _value);
				void onCallbackListValidate();
				void onCallbackHomePressed();
			protected:
				virtual void onChangePropertyPath();
				virtual void onChangePropertyFile();
				virtual void onChangePropertyLabelTitle();
				virtual void onChangePropertyLabelValidate();
				virtual void onChangePropertyLabelCancel();
		};
	};
};

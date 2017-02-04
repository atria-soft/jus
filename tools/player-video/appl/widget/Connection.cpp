/** @file
 * @author Edouard DUPIN
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */


#include <ewol/widget/meta/Connection.hpp>
#include <ewol/widget/Sizer.hpp>
#include <ewol/widget/List.hpp>
#include <ewol/widget/Button.hpp>
#include <ewol/widget/CheckBox.hpp>
#include <ewol/widget/ListFileSystem.hpp>
#include <ewol/widget/Entry.hpp>
#include <ewol/widget/Spacer.hpp>
#include <ewol/widget/Image.hpp>
#include <ewol/widget/Composer.hpp>
#include <ewol/widget/Manager.hpp>
//#include <vector>
#include <vector>
#include <etk/tool.hpp>
#include <etk/os/FSNode.hpp>

extern "C" {
	// file browsing ...
	#include <dirent.h>
}

#include <ewol/ewol.hpp>

appl::widget::Connection::Connection() :
  signalCancel(this, "cancel", ""),
  signalValidate(this, "validate", "") {
	addObjectType("appl::widget::Connection");
}

void appl::widget::Connection::init(ememory::SharedPtr<appl::ClientProperty> _baseProperty) {
	appl::widget::Composer::init();
	m_baseProperty = _baseProperty;
	loadFromFile("DATA:gui-connection.xml", getId());
	subBind(appl::widget::CheckBox, "[" + etk::to_string(getId()) + "]file-shooser:show-hiden-file", signalValue, sharedFromThis(), &appl::widget::Connection::onCallbackHidenFileChangeChangeValue);
	subBind(appl::widget::Button, "[" + etk::to_string(getId()) + "]file-shooser:button-validate", signalPressed, sharedFromThis(), &appl::widget::Connection::onCallbackListValidate);
	subBind(appl::widget::Button, "[" + etk::to_string(getId()) + "]file-shooser:button-cancel", signalPressed, sharedFromThis(), &appl::widget::Connection::onCallbackButtonCancelPressed);
	subBind(appl::widget::ListFileSystem, "[" + etk::to_string(getId()) + "]file-shooser:list-folder", signalFolderValidate, sharedFromThis(), &appl::widget::Connection::onCallbackListFolderSelectChange);
	subBind(appl::widget::ListFileSystem, "[" + etk::to_string(getId()) + "]file-shooser:list-files", signalFileSelect, sharedFromThis(), &appl::widget::Connection::onCallbackListFileSelectChange);
	subBind(appl::widget::ListFileSystem, "[" + etk::to_string(getId()) + "]file-shooser:list-files", signalFileValidate, sharedFromThis(), &appl::widget::Connection::onCallbackListFileValidate);
	subBind(appl::widget::Entry, "[" + etk::to_string(getId()) + "]file-shooser:entry-file", signalModify, sharedFromThis(), &appl::widget::Connection::onCallbackEntryFileChangeValue);
	subBind(appl::widget::Entry, "[" + etk::to_string(getId()) + "]file-shooser:entry-file", signalEnter, sharedFromThis(), &appl::widget::Connection::onCallbackListFileValidate);
	subBind(appl::widget::Entry, "[" + etk::to_string(getId()) + "]file-shooser:entry-folder", signalModify, sharedFromThis(), &appl::widget::Connection::onCallbackEntryFolderChangeValue);
	//composerBind(appl::widget::CheckBox, "[" + etk::to_string(getId()) + "]file-shooser:entry-folder", signalEnter, sharedFromThis(), &appl::widget::Connection::);
	subBind(appl::widget::Image, "[" + etk::to_string(getId()) + "]file-shooser:img-home", signalPressed, sharedFromThis(), &appl::widget::Connection::onCallbackHomePressed);
	// set the default Folder properties:
	updateCurrentFolder();
	propertyCanFocus.set(true);
}

void appl::widget::Connection::onGetFocus() {
	// transfert focus on a specific widget...
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:entry-file", "focus", "true");
}

appl::widget::Connection::~Connection() {
	
}

void appl::widget::Connection::onChangePropertyPath() {
	propertyPath.getDirect() = *propertyPath + "/";
	updateCurrentFolder();
}

void appl::widget::Connection::onChangePropertyFile() {
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:entry-file", "value", propertyFile);
	//updateCurrentFolder();
}

void appl::widget::Connection::onChangePropertyLabelTitle() {
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:title-label", "value", propertyLabelTitle);
}

void appl::widget::Connection::onChangePropertyLabelValidate() {
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:validate-label", "value", propertyLabelValidate);
}

void appl::widget::Connection::onChangePropertyLabelCancel() {
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:cancel-label", "value", propertyLabelCancel);
}

void appl::widget::Connection::onCallbackEntryFolderChangeValue(const std::string& _value) {
	// == > change the folder name
	// TODO : change the folder, if it exit ...
}

void appl::widget::Connection::onCallbackEntryFileChangeValue(const std::string& _value) {
	// == > change the file name
	propertyFile.setDirect(_value);
	// update the selected file in the list :
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:list-files", "select", propertyFile);
}

void appl::widget::Connection::onCallbackButtonCancelPressed() {
	// == > Auto remove ...
	signalCancel.emit();
	autoDestroy();
}

void appl::widget::Connection::onCallbackHidenFileChangeChangeValue(const bool& _value) {
	if (_value == true) {
		propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:list-folder", "show-hidden", "true");
		propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:list-files", "show-hidden", "true");
	} else {
		propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:list-folder", "show-hidden", "false");
		propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:list-files", "show-hidden", "false");
	}
}

void appl::widget::Connection::onCallbackListFolderSelectChange(const std::string& _value) {
	// == > this is an internal event ...
	EWOL_DEBUG(" old PATH: '" << *propertyPath << "' + '" << _value << "'");
	propertyPath.setDirect(propertyPath.get() + _value);
	EWOL_DEBUG("new PATH: '" << *propertyPath << "'");
	propertyPath.setDirect(etk::simplifyPath(*propertyPath));
	propertyFile.setDirect("");
	updateCurrentFolder();
}

void appl::widget::Connection::onCallbackListFileSelectChange(const std::string& _value) {
	propertyFile.set(_value);
	/*
	std::string tmpFileCompleatName = m_folder;
	tmpFileCompleatName += m_file;
	// TODO : generateEventId(_msg.getMessage(), tmpFileCompleatName);
	*/
}

void appl::widget::Connection::onCallbackListFileValidate(const std::string& _value) {
	// select the file  == > generate a validate
	propertyFile.set(_value);
	EWOL_VERBOSE(" generate a fiel opening : '" << propertyPath << "' / '" << propertyFile << "'");
	signalValidate.emit(getCompleateFileName());
	autoDestroy();
}

void appl::widget::Connection::onCallbackListValidate() {
	if (propertyFile.get() == "") {
		EWOL_WARNING(" Validate : '" << propertyPath << "' / '" << propertyFile << "' ==> error No name ...");
		return;
	}
	EWOL_DEBUG(" generate a file opening : '" << propertyPath << "' / '" << propertyFile << "'");
	signalValidate.emit(getCompleateFileName());
	autoDestroy();
}

void appl::widget::Connection::onCallbackHomePressed() {
	std::string tmpUserFolder = etk::getUserHomeFolder();
	EWOL_DEBUG("new PATH : \"" << tmpUserFolder << "\"");
	
	propertyPath.setDirect(etk::simplifyPath(tmpUserFolder));
	
	propertyFile.setDirect("");
	updateCurrentFolder();
}

void appl::widget::Connection::updateCurrentFolder() {
	if (*propertyPath != "") {
		if (propertyPath.get()[propertyPath->size()-1] != '/') {
			propertyPath.getDirect() +=  "/";
		}
	}
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:list-files", "path", propertyPath);
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:list-folder", "path", propertyPath);
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]file-shooser:entry-folder", "value", propertyPath);
	markToRedraw();
}

std::string appl::widget::Connection::getCompleateFileName() {
	std::string tmpString = propertyPath;
	tmpString += "/";
	tmpString += propertyFile;
	etk::FSNode node(tmpString);
	return node.getName();
}

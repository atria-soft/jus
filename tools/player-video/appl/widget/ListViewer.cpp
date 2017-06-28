/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#include <appl/debug.hpp>
#include <ewol/widget/Widget.hpp>



#include <appl/debug.hpp>
#include <appl/widget/ListViewer.hpp>
#include <ewol/object/Manager.hpp>
#include <etk/tool.hpp>

#include <egami/egami.hpp>
#include <zeus/zeus.hpp>
#include <zeus/Client.hpp>
#include <zeus/service/ProxyVideo.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/ObjectRemote.hpp>
#include <echrono/Steady.hpp>
#include <zeus/FutureGroup.hpp>
#include <etk/stdTools.hpp>
#include <zeus/ProxyMedia.hpp>
#include <ejson/ejson.hpp>

appl::widget::ListViewer::ListViewer() :
  signalSelect(this, "select", "Select a media to view") {
	addObjectType("appl::widget::ListViewer");
	propertyCanFocus.setDirectCheck(true);
	setLimitScrolling(vec2(1.0f,0.2f));
	setMouseLimit(1);
}

void appl::widget::ListViewer::init() {
	ewol::Widget::init();
	markToRedraw();
	//m_compImageVideo.setSource("DATA:Video.svg", 128);
	//m_compImageAudio.setSource("DATA:MusicNote.svg", 128);
}

appl::widget::ListViewer::~ListViewer() {
	
}

void appl::widget::ListViewer::searchElements(std::string _filter) {
	if (_filter == "group") {
		searchElementsInternal("*", "type");
	} else if (_filter == "film") {
		searchElementsInternal("'type' == 'film' AND 'production-methode' == 'picture'");
	} else if (_filter == "annimation") {
		searchElementsInternal("'type' == 'film' AND 'production-methode' == 'draw'");
	} else if (_filter == "tv-show") {
		searchElementsInternal("'type' == 'tv-show' AND 'production-methode' == 'picture'", "series-name");
	} else if (_filter == "tv-annimation") {
		searchElementsInternal("'type' == 'tv-show' AND 'production-methode' == 'draw'", "series-name");
	} else if (_filter == "theater") {
		searchElementsInternal("'type' == 'theater'");
	} else if (_filter == "one-man") {
		searchElementsInternal("'type' == 'one-man'");
	} else if (_filter == "courses") {
		searchElementsInternal("'type' == 'film' AND 'production-methode' == 'short'");
	} else {
		searchElementsInternal("*");
	}
}

void appl::ElementProperty::loadData() {
	// Check progression status:
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_metadataUpdated != appl::statusLoadingData::noData) {
			return;
		}
		m_metadataUpdated = appl::statusLoadingData::inProgress;
	}
	int32_t nbCallOfMetaData = 8;
	auto tmpProperty = sharedFromThis();
	// Get the media
	zeus::Future<zeus::ProxyMedia> futMedia = m_remoteServiceVideo.get(m_id);
	futMedia.andElse([=](const std::string& _error, const std::string& _help) mutable {
	                 	APPL_INFO("    [" << tmpProperty->m_id << "] get media error: " << tmpProperty->m_id);
	                 	{
	                 		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 		tmpProperty->m_title = "[ERROR] can not get media informations <br/>" + _error + ": " + _help;
	                 		tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	}
	                 	m_widget->markToRedraw();
	                 	return true;
	                 });
	futMedia.andThen([=](zeus::ProxyMedia _media) mutable {
	                 	APPL_INFO("    [" << tmpProperty->m_id << "] get media: " << tmpProperty->m_id);
	                 	if (_media.exist() == false) {
	                 		APPL_ERROR("get media error");
	                 		{
	                 			std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 			tmpProperty->m_title = "[ERROR] can not get media informations (2)";
	                 			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 		}
	                 		m_widget->markToRedraw();
	                 		return true;
	                 	}
	                 	_media.getMetadata("title")
	                 	    .andElse([=](const std::string& _error, const std::string& _help) mutable {
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             })
	                 	    .andThen([=](std::string _value) mutable {
	                 	             	APPL_INFO("    [" << tmpProperty->m_id << "] get title: " << _value);
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_title = _value;
	                 	             	}
	                 	             	m_widget->markToRedraw();
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             });
	                 	_media.getMetadata("series-name")
	                 	    .andElse([=](const std::string& _error, const std::string& _help) mutable {
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             })
	                 	    .andThen([=](std::string _value) mutable {
	                 	             	APPL_INFO("    [" << tmpProperty->m_id << "] get serie: " << _value);
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_serie = _value;
	                 	             	}
	                 	             	m_widget->markToRedraw();
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             });
	                 	_media.getMetadata("saison")
	                 	    .andElse([=](const std::string& _error, const std::string& _help) mutable {
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             })
	                 	    .andThen([=](std::string _value) mutable {
	                 	             	APPL_INFO("    [" << tmpProperty->m_id << "] get saison: " << _value);
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_saison = _value;
	                 	             	}
	                 	             	m_widget->markToRedraw();
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             });
	                 	_media.getMetadata("episode")
	                 	    .andElse([=](const std::string& _error, const std::string& _help) mutable {
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             })
	                 	    .andThen([=](std::string _value) mutable {
	                 	             	APPL_INFO("    [" << tmpProperty->m_id << "] get episode: " << _value);
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_episode = _value;
	                 	             	}
	                 	             	m_widget->markToRedraw();
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             });
	                 	_media.getMetadata("description")
	                 	    .andElse([=](const std::string& _error, const std::string& _help) mutable {
	                 	             	APPL_INFO("Get remot error : " << _error << " " << _help);
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             })
	                 	    .andThen([=](std::string _value) mutable {
	                 	             	APPL_INFO("    [" << tmpProperty->m_id << "] get description: " << _value);
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_description = _value;
	                 	             	}
	                 	             	m_widget->markToRedraw();
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             });
	                 	_media.getMetadata("production-methode")
	                 	    .andElse([=](const std::string& _error, const std::string& _help) mutable {
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             })
	                 	    .andThen([=](std::string _value) mutable {
	                 	             	APPL_INFO("    [" << tmpProperty->m_id << "] get production-methode: " << _value);
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_productMethode = _value;
	                 	             	}
	                 	             	m_widget->markToRedraw();
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             });
	                 	_media.getMetadata("type")
	                 	    .andElse([=](const std::string& _error, const std::string& _help) mutable {
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             })
	                 	    .andThen([=](std::string _value) mutable {
	                 	             	APPL_INFO("    [" << tmpProperty->m_id << "] get type: " << _value);
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_type = _value;
	                 	             	}
	                 	             	m_widget->markToRedraw();
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             });
	                 	_media.getMineType()
	                 	    .andElse([=](const std::string& _error, const std::string& _help) mutable {
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             })
	                 	    .andThen([=](std::string _value) mutable {
	                 	             	APPL_INFO("    [" << tmpProperty->m_id << "] get mine-type: " << _value);
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_mineType = _value;
	                 	             		if (etk::start_with(tmpProperty->m_mineType, "video") == true) {
	                 	             			// TODO : Optimise this ...
	                 	             			tmpProperty->m_thumb = egami::load("DATA:Video.svg", ivec2(128,128));
	                 	             		} else if (etk::start_with(tmpProperty->m_mineType, "audio") == true) {
	                 	             			// TODO : Optimise this ...
	                 	             			tmpProperty->m_thumb = egami::load("DATA:MusicNote.svg", ivec2(128,128));
	                 	             		}
	                 	             	}
	                 	             	m_widget->markToRedraw();
	                 	             	{
	                 	             		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 	             		tmpProperty->m_nbElementLoaded++;
	                 	             		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                 	             			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	             		}
	                 	             	}
	                 	             	return true;
	                 	             });
	                 	//elem->m_thumb = remoteServiceVideo.mediaThumbGet(it, 128).wait().get();
	                 	return true;
	                 });
	auto futMediaCover = m_remoteServiceVideo.getCover(m_id, 128);
	futMediaCover.andElse([=](const std::string& _error, const std::string& _help) mutable {
	                      	APPL_INFO("    [" << tmpProperty->m_id << "] get cover error: " << tmpProperty->m_id << ": " << _help);
	                      	// TODO : Remove this ...
	                      	std::this_thread::sleep_for(std::chrono::milliseconds(400));
	                      	std::string serie;
	                      	{
	                      		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                      		serie = tmpProperty->m_serie;
	                      	}
	                      	if (serie != "") {
	                      		auto futMediaGroupCover = m_remoteServiceVideo.getGroupCover(serie, 128);
	                      		futMediaGroupCover.andElse([=](const std::string& _error, const std::string& _help) mutable {
	                      		                        	APPL_INFO("    [" << tmpProperty->m_id << "] get cover Group error: " << serie << ": " << _help);
	                      		                        	{
	                      		                        		m_widget->markToRedraw();
	                      		                        		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                      		                        		tmpProperty->m_nbElementLoaded++;
	                      		                        		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                      		                        			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                      		                        		}
	                      		                        	}
	                      		                        	return true;
	                      		                        });
	                      		futMediaGroupCover.andThen([=](zeus::ProxyFile _media) mutable {
	                      		                        	APPL_INFO("    [" << tmpProperty->m_id << "] get cover Group on: " << serie);
	                      		                        	auto mineTypeFut = _media.getMineType();
	                      		                        	std::vector<uint8_t> bufferData = zeus::storeInMemory(_media);
	                      		                        	APPL_INFO("    [" << tmpProperty->m_id << "] get cover Group on: " << serie << " store in memory " << bufferData.size());
	                      		                        	std::string mineType = mineTypeFut.wait().get();
	                      		                        	APPL_INFO("    [" << tmpProperty->m_id << "] get cover Group on: " << serie << " mineType '" << mineType << "'");
	                      		                        	{
	                      		                        		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                      		                        		tmpProperty->m_thumb = egami::load(mineType, bufferData);
	                      		                        		tmpProperty->m_thumbPresent = true;
	                      		                        	}
	                      		                        	APPL_WARNING("Get the Thumb ... " << tmpProperty->m_title << " ==> " << tmpProperty->m_thumb);
	                      		                        	m_widget->markToRedraw();
	                      		                        	{
	                      		                        		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                      		                        		tmpProperty->m_nbElementLoaded++;
	                      		                        		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                      		                        			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                      		                        		}
	                      		                        	}
	                      		                        	return true;
	                      		                        });
	                      	} else {
	                      		m_widget->markToRedraw();
	                      		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                      		tmpProperty->m_nbElementLoaded++;
	                      		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                      			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                      		}
	                      	}
	                      	return true;
	                      });
	futMediaCover.andThen([=](zeus::ProxyFile _media) mutable {
	                      	APPL_INFO("    [" << tmpProperty->m_id << "] get cover on: " << tmpProperty->m_id);
	                      	auto mineTypeFut = _media.getMineType();
	                      	std::vector<uint8_t> bufferData = zeus::storeInMemory(_media);
	                      	APPL_INFO("    [" << tmpProperty->m_id << "] get cover on: " << tmpProperty->m_id << " store in memory " << bufferData.size());
	                      	std::string mineType = mineTypeFut.wait().get();
	                      	APPL_INFO("    [" << tmpProperty->m_id << "] get cover on: " << tmpProperty->m_id << " mineType '" << mineType << "'");
	                      	{
	                      		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                      		tmpProperty->m_thumb = egami::load(mineType, bufferData);
	                      		tmpProperty->m_thumbPresent = true;
	                      	}
	                      	APPL_WARNING("Get the Thumb ... " << tmpProperty->m_title << " ==> " << tmpProperty->m_thumb);
	                      	m_widget->markToRedraw();
	                      	{
	                      		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                      		tmpProperty->m_nbElementLoaded++;
	                      		if (tmpProperty->m_nbElementLoaded >= nbCallOfMetaData) {
	                      			tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                      		}
	                      	}
	                      	return true;
	                      });
}

bool appl::ElementProperty::LoadDataEnded() {
	std::unique_lock<std::mutex> lock(m_mutex);
	return m_metadataUpdated == appl::statusLoadingData::done;
}
void appl::ElementPropertyGroup::loadData() {
	// Check progression status:
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_metadataUpdated != appl::statusLoadingData::noData) {
			return;
		}
		m_metadataUpdated = appl::statusLoadingData::inProgress;
	}
	auto tmpProperty = sharedFromThis();
	// Get the media
	auto futMedia = m_remoteServiceVideo.getGroupCover(m_title, 128);
	futMedia.andElse([=](const std::string& _error, const std::string& _help) mutable {
	                 	APPL_INFO("    [" << tmpProperty->m_id << "] get cover error on group: " << tmpProperty->m_title << ": " << _help);
	                 	{
	                 		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 		tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	}
	                 	m_widget->markToRedraw();
	                 	return true;
	                 });
	futMedia.andThen([=](zeus::ProxyFile _media) mutable {
	                 	APPL_INFO("    [" << tmpProperty->m_id << "] get cover on group: " << tmpProperty->m_title);
	                 	auto mineTypeFut = _media.getMineType();
	                 	std::vector<uint8_t> bufferData = zeus::storeInMemory(_media);
	                 	APPL_INFO("    [" << tmpProperty->m_id << "] get cover on group: " << tmpProperty->m_title << " store in memory " << bufferData.size());
	                 	std::string mineType = mineTypeFut.wait().get();
	                 	APPL_INFO("    [" << tmpProperty->m_id << "] get cover on group: " << tmpProperty->m_title << " mineType '" << mineType << "'");
	                 	{
	                 		std::unique_lock<std::mutex> lock(tmpProperty->m_mutex);
	                 		tmpProperty->m_thumb = egami::load(mineType, bufferData);
	                 		tmpProperty->m_thumbPresent = true;
	                 	}
	                 	APPL_WARNING("Get the Thumb ... " << tmpProperty->m_title << " ==> " << tmpProperty->m_thumb);
	                 	m_widget->markToRedraw();
	                 	tmpProperty->m_metadataUpdated = appl::statusLoadingData::done;
	                 	return true;
	                 });
}

bool appl::ElementPropertyGroup::LoadDataEnded() {
	std::unique_lock<std::mutex> lock(m_mutex);
	return m_metadataUpdated == appl::statusLoadingData::done;
}

void appl::widget::ListViewer::backHistory() {
	if (m_history.size() == 0) {
		// nothing to do ...
		return;
	}
	auto value = m_history.back();
	m_history.pop_back();
	searchElementsInternal(value.first, value.second, false);
}

void appl::widget::ListViewer::searchElementsInternal(const std::string& _filter, const std::string& _group, bool _storeHistory) {
	m_listElement.clear();
	m_listElementGroup.clear();
	m_listDisplay.clear();
	resetScrollOrigin();
	if (m_clientProp == nullptr) {
		APPL_ERROR("No client Availlable ...");
		return;
	}
	if (    m_currentFilter != ""
	     && _storeHistory == true) {
		m_history.push_back(std::make_pair(m_currentFilter, m_currentGroup));
	}
	m_currentFilter = _filter;
	m_currentGroup = _group;
	markToRedraw();
	m_clientProp->connect();
	if (m_clientProp->getConnection().isAlive() == false) {
		APPL_ERROR("Conection is not alive anymore ...");
		return;
	}
	
	bool retSrv = m_clientProp->getConnection().waitForService("video");
	if (retSrv == false) {
		APPL_ERROR(" ==> SERVICE not availlable or not started");
		return;
	}
	// get all the data:
	zeus::service::ProxyVideo remoteServiceVideo = m_clientProp->getConnection().getService("video");
	// remove all media (for test)
	if (remoteServiceVideo.exist() == false) {
		APPL_ERROR("    ==> Service does not exist : 'video'");
		return;
	}
	if (m_currentGroup != "") {
		zeus::Future<std::vector<std::string>> listElem = remoteServiceVideo.getMetadataValuesWhere(m_currentGroup, m_currentFilter).wait();
		if (listElem.hasError() == true) {
			APPL_ERROR("    ==> Can not get elements from video service <with fileter ! '" << m_currentFilter << "' : " << listElem.getErrorType() << " : " << listElem.getErrorHelp());
			return;
		}
		std::vector<std::string> returnValues = listElem.get();
		APPL_INFO("Get some Values: " << returnValues << "");
		if (returnValues.size() == 1) {
			// TODO: if we have a single element and this is requested as a group ==> just jump in it
			
		}
		for (auto &it : returnValues) {
			auto elem = ememory::makeShared<ElementPropertyGroup>(remoteServiceVideo, ememory::staticPointerCast<ewol::Widget>(sharedFromThis()));
			if (elem == nullptr) {
				APPL_ERROR("Can not allocate element... " << it);
				continue;
			}
			elem->m_id = 0;
			elem->m_filter = it;
			if (m_currentGroup == "saison") {
				elem->m_title = TRANSLATE("_T{saison} ") + it;
			} else {
				elem->m_title = it;
			}
			//elem->m_thumb = remoteServiceVideo.mediaThumbGet(it, 128).wait().get();
			m_listElementGroup.push_back(elem);
		}
	} else {
		zeus::Future<std::vector<uint32_t>> listElem = remoteServiceVideo.getSQL(m_currentFilter).wait();
		if (listElem.hasError() == true) {
			APPL_ERROR("    ==> Can not get element from video service <with fileter ! '" << m_currentFilter << "' : " << listElem.getErrorType() << " : " << listElem.getErrorHelp());
			return;
		}
		std::vector<uint32_t> returnValues = listElem.get();
		APPL_INFO("Get some Values: " << returnValues << "");
		for (auto &it : returnValues) {
			auto elem = ememory::makeShared<ElementProperty>(remoteServiceVideo, ememory::staticPointerCast<ewol::Widget>(sharedFromThis()));
			if (elem == nullptr) {
				APPL_ERROR("Can not allocate element... " << it);
				continue;
			}
			elem->m_id = it;
			// ==> loading is done when display is requested ...
			m_listElement.push_back(elem);
		}
	}
	APPL_INFO("Request All is done");
}

void appl::widget::ListViewer::onDraw() {
	m_text.draw();
	for (auto &it : m_listDisplay) {
		if (it == nullptr) {
			continue;
		}
		it->draw();
	}
	ewol::widget::WidgetScrolled::onDraw();
}

void appl::widget::ListViewer::onRegenerateDisplay() {
	ewol::widget::WidgetScrolled::onRegenerateDisplay();
	//!< Check if we really need to redraw the display, if not needed, we redraw the previous data ...
	if (needRedraw() == false) {
		return;
	}
	std::u32string errorString = U"No element Availlable";
	
	m_text.clear();
	// to know the size of one line : 
	vec3 minSize = m_text.calculateSize(char32_t('A'));
	if (    m_listElement.size() == 0
	     && m_listElementGroup.size() == 0) {
		int32_t paddingSize = 2;
		
		vec2 tmpMax = propertyMaxSize->getPixel();
		
		/*
		if (tmpMax.x() <= 999999) {
			m_text.setTextAlignement(0, tmpMax.x()-2*paddingSize, ewol::compositing::alignLeft);
		}
		*/
		vec3 curentTextSize = m_text.calculateSizeDecorated(errorString);
		
		ivec2 localSize = m_minSize;
		
		// no change for the text orogin:
		vec3 tmpTextOrigin((m_size.x() - m_minSize.x()) / 2.0,
		                   (m_size.y() - m_minSize.y()) / 2.0,
		                   0);
		
		if (propertyFill->x() == true) {
			localSize.setX(m_size.x());
			tmpTextOrigin.setX(0);
		}
		if (propertyFill->y() == true) {
			localSize.setY(m_size.y());
			tmpTextOrigin.setY(m_size.y() - 2*paddingSize - curentTextSize.y());
		}
		tmpTextOrigin += vec3(paddingSize, paddingSize, 0);
		localSize -= vec2(2*paddingSize,2*paddingSize);
		
		tmpTextOrigin.setY( tmpTextOrigin.y() + (m_minSize.y()-2*paddingSize) - minSize.y());
		
		vec2 textPos(tmpTextOrigin.x(), tmpTextOrigin.y());
		
		vec3 drawClippingPos(paddingSize, paddingSize, -0.5);
		vec3 drawClippingSize((m_size.x() - paddingSize),
		                      (m_size.y() - paddingSize),
		                      1);
		
		// clean the element
		m_text.reset();
		m_text.setDefaultColorFg(etk::color::red);
		m_text.setPos(tmpTextOrigin);
		//APPL_INFO("Regenerate display : " << tmpTextOrigin << "  " << m_origin << "  " << m_size);
		//APPL_VERBOSE("[" << getId() << "] {" << errorString << "} display at pos : " << tmpTextOrigin);
		m_text.setTextAlignement(tmpTextOrigin.x(), tmpTextOrigin.x()+localSize.x(), ewol::compositing::alignLeft);
		m_text.setClipping(drawClippingPos, drawClippingSize);
		m_text.printDecorated(errorString);
		// call the herited class...
		WidgetScrolled::onRegenerateDisplay();
	}
	
	// Here the real Display get a square in pixel of 2cm x 2cm:
	vec2 realPixelSize = gale::Dimension(vec2(3,3), gale::distance::centimeter).getPixel();
	// TODO : Understand Why this not work ...
	realPixelSize = vec2(minSize.y()*4,minSize.y()*4+6); // add arbitrary 6 pixel ...
	// This will generate the number of element that can be displayed:
	int32_t verticalNumber = m_size.y() / realPixelSize.y() + 2; // +1 for the not entire view of element and +1 for the moving element view ...
	//verticalNumber = 10;
	//       APPL_WARNING("We can see " << verticalNumber << " elements: " << realPixelSize);
	// TODO : Remove this ... bad optim ...
	//m_listDisplay.clear();
	if (m_listDisplay.size() != verticalNumber) {
		m_listDisplay.clear();
		for (size_t iii=0; iii<verticalNumber; ++iii) {
			auto elem = ememory::makeShared<ElementDisplayed>();
			m_listDisplay.push_back(elem);
		}
	}
	int32_t offset = m_originScrooled.y() / realPixelSize.y();
	APPL_VERBOSE("origin scrolled : " << m_originScrooled << " nb Pixel/element = " << realPixelSize.y() << " offset=" << offset);
	for (size_t iii=0; iii<m_listDisplay.size(); ++iii) {
		auto elem = m_listDisplay[iii];
		if (elem != nullptr) {
			elem->m_idCurentElement = offset + iii;
			if (offset + iii < m_listElement.size()) {
				elem->m_property = m_listElement[offset + iii];
				if (elem->m_property != nullptr) {
					elem->m_property->loadData();
				}
			} else {
				elem->m_property.reset();
			}
			if (offset + iii < m_listElementGroup.size()) {
				elem->m_propertyGroup = m_listElementGroup[offset + iii];
				if (elem->m_propertyGroup != nullptr) {
					elem->m_propertyGroup->loadData();
				}
			} else {
				elem->m_propertyGroup.reset();
			}
			//switch(iii%6) {
			switch((offset + iii)%6) {
				case 0:
					elem->m_bgColor = etk::color::red;
					break;
				case 1:
					elem->m_bgColor = etk::color::green;
					break;
				case 2:
					elem->m_bgColor = etk::color::orange;
					break;
				case 3:
					elem->m_bgColor = etk::color::purple;
					break;
				case 4:
					elem->m_bgColor = etk::color::gray;
					break;
				case 5:
					elem->m_bgColor = etk::color::cyan;
					break;
				default:
					elem->m_bgColor = etk::color::blue;
					break;
			}
		} else {
			APPL_ERROR("create nullptr");
		}
	}
	
	//display only ofsetted element
	float realOffset = float(int32_t(m_originScrooled.y() / realPixelSize.y())) * realPixelSize.y();
	// Now we request display of the elements:
	vec2 elementSize = vec2(m_size.x(), int32_t(realPixelSize.y()));
	vec2 startPos = vec2(-m_originScrooled.x(), m_size.y()) - vec2(0, elementSize.y()-(m_originScrooled.y()-realOffset));
	for (auto &it : m_listDisplay) {
		if (it == nullptr) {
			startPos -= vec2(0, elementSize.y());
			continue;
		}
		if (    startPos.y() - elementSize.y() <= m_size.y()
		     && startPos.y() + elementSize.y() >= 0) {
			it->generateDisplay(startPos, elementSize);
		} else {
			it->clear();
		}
		startPos -= vec2(0, elementSize.y());
	}
	m_maxSize.setX(m_size.x());
	m_maxSize.setY(float(std::max(m_listElement.size(),m_listElementGroup.size()))*elementSize.y());
	// call the herited class...
	ewol::widget::WidgetScrolled::onRegenerateDisplay();
}

/*
		public:
			ememory::SharedPtr<appl::ElementProperty> m_property;
			int32_t m_idCurentElement;
			etk::Color<float> m_bgColor;
			ewol::compositing::Image m_image;
			ewol::compositing::Text m_text;
			ewol::compositing::Drawing m_draw;
*/

void appl::ElementDisplayed::generateDisplay(vec2 _startPos, vec2 _size) {
	m_image.clear();
	m_text.clear();
	m_draw.clear();
	if (    m_property == nullptr
	     && m_propertyGroup == nullptr) {
		return;
	}
	//APPL_INFO("Regenrate size :  " << _startPos << " " << _size);
	// --------------------------------------------
	// -- Create a simple border
	// --------------------------------------------
	_startPos += vec2(3,3);
	_size -= vec2(3,3)*2;
	m_draw.setPos(_startPos);
	m_draw.setColor(etk::color::black);
	m_draw.rectangleWidth(_size);
	_startPos += vec2(2,2);
	_size -= vec2(2,2)*2;
	m_draw.setPos(_startPos);
	m_draw.setColor(m_bgColor);
	m_draw.rectangleWidth(_size);
	
	m_pos = _startPos;
	m_size = _size;
	
	// --------------------------------------------
	// -- Display text...
	// --------------------------------------------
	m_text.reset();
	m_text.setDefaultColorFg(etk::color::black);
	vec3 minSize = m_text.calculateSize(char32_t('A'));
	float borderOffset = 4;
	vec2 originText = _startPos + vec2(_size.y()+borderOffset, _size.y()-minSize.y() - borderOffset);
	m_text.setPos(originText);
	//APPL_INFO("Regenerate display : " << tmpTextOrigin << "  " << m_origin << "  " << m_size);
	//APPL_VERBOSE("[" << getId() << "] {" << errorString << "} display at pos : " << tmpTextOrigin);
	m_text.setTextAlignement(originText.x(), originText.x()+_size.x()-_size.y(), ewol::compositing::alignDisable);
	// TODO: m_text.setClipping(originText, vec2(originText.x()+_size.x()-_size.y(), _size.y()));
	std::string textToDisplay;
	if (m_property != nullptr) {
		if (m_property->LoadDataEnded() == false) {
			textToDisplay += "<br/><i>Loading in progress</i> ... " + etk::to_string(m_property->m_nbElementLoaded) + "/8";
		} else {
			std::unique_lock<std::mutex> lock(m_property->m_mutex);
			//m_text.setClipping(drawClippingPos, drawClippingSize);
			textToDisplay = "<b>" + m_property->m_title + "</b><br/>";
			bool newLine = false;
			if (m_property->m_serie != "") {
				textToDisplay += "<i>Serie: <b>" + m_property->m_serie + "</b></i><br/>";
			}
			if (m_property->m_saison != "") {
				textToDisplay += "<i>Saison: <b>" + m_property->m_saison + "</b></i> ";
				newLine = true;
			}
			if (m_property->m_episode != "") {
				textToDisplay += "<i>Episode: <b>" + m_property->m_episode + "</b></i> ";
				newLine = true;
			}
			if (m_property->m_type != "") {
				textToDisplay += "    <i>type: <b>" + m_property->m_type + "</b></i> ";
				if (m_property->m_productMethode != "") {
					textToDisplay += " / " + m_property->m_productMethode + " ";
				}
				newLine = true;
			}
			if (newLine == true) {
				textToDisplay += "<br/>";
			}
			textToDisplay += "<i>" + m_property->m_description + "</i>";
		}
	} else {
		if (m_propertyGroup->LoadDataEnded() == false) {
			textToDisplay += "<br/><b>" + m_propertyGroup->m_title + "</b><br/><i>Loading in progress</i> ...";
		} else {
			//m_text.setClipping(drawClippingPos, drawClippingSize);
			textToDisplay = "<br/><b>" + m_propertyGroup->m_title + "</b><br/>";
		}
	}
	m_text.printDecorated(textToDisplay);
	
	// --------------------------------------------
	// -- Display Image...
	// --------------------------------------------
	if (m_property != nullptr) {
		bool haveThumb = false;
		if (m_property->LoadDataEnded() == false) {
			haveThumb = false;
			m_image.setSource("DATA:Home.svg", 128);
		} else {
			std::unique_lock<std::mutex> lock(m_property->m_mutex);
			if (m_property->m_thumbPresent == true) {
				haveThumb = true;
				m_image.setSource(m_property->m_thumb);
				m_image.setPos(_startPos);
				m_image.print(vec2(_size.y(), _size.y()));
			} else {
				if (etk::start_with(m_property->m_mineType, "video") == true) {
					m_image.setSource("DATA:Video.svg", 128);
				} else if (etk::start_with(m_property->m_mineType, "audio") == true) {
					m_image.setSource("DATA:MusicNote.svg", 128);
				} else {
					APPL_DEBUG("Set image: Unknow type '" << m_property->m_mineType << "'");
					m_image.setSource("DATA:Home.svg", 128);
				}
			}
		}
		if (haveThumb == false) {
			m_image.setPos(_startPos+vec2(10,10));
			m_image.print(vec2(_size.y(), _size.y())-vec2(20,20));
		}
	} else {
		bool haveThumb = false;
		if (m_propertyGroup->LoadDataEnded() == false) {
			haveThumb = false;
		} else {
			std::unique_lock<std::mutex> lock(m_propertyGroup->m_mutex);
			if (m_propertyGroup->m_thumbPresent == true) {
				haveThumb = true;
				m_image.setSource(m_propertyGroup->m_thumb);
				m_image.setPos(_startPos);
				m_image.print(vec2(_size.y(), _size.y()));
			}
		}
		if (haveThumb == false) {
			m_image.setSource("DATA:Home.svg", 128);
			m_image.setPos(_startPos+vec2(10,10));
			m_image.print(vec2(_size.y(), _size.y())-vec2(20,20));
		}
	}
	
}


bool appl::widget::ListViewer::onEventInput(const ewol::event::Input& _event) {
	if (ewol::widget::WidgetScrolled::onEventInput(_event) == true) {
		return true;
	}
	APPL_VERBOSE("Event on BT : " << _event << " size=" << m_size << " maxSize=" << m_maxSize);
	vec2 relativePos = relativePosition(_event.getPos());
	int32_t findId = -1;
	for (size_t iii=0; iii<m_listDisplay.size(); ++iii) {
		if (m_listDisplay[iii] == nullptr) {
			continue;
		}
		if(    relativePos.x() < m_listDisplay[iii]->m_pos.x()
		    || relativePos.y() < m_listDisplay[iii]->m_pos.y()
		    || relativePos.x() > m_listDisplay[iii]->m_pos.x() + m_listDisplay[iii]->m_size.x()
		    || relativePos.y() > m_listDisplay[iii]->m_pos.y() + m_listDisplay[iii]->m_size.y() ) {
			continue;
		}
		findId = iii;
		break;
	}
	if (findId == -1) {
		return false;
	}
	if (_event.getId() == 1) {
		if(_event.getStatus() == gale::key::status::pressSingle) {
			APPL_DEBUG("Select element : " << findId << "  " << m_listDisplay[findId]->m_idCurentElement);
			if (m_listDisplay[findId]->m_property != nullptr) {
				ememory::SharedPtr<appl::ElementProperty> prop = m_listDisplay[findId]->m_property;
				if (prop == nullptr) {
					return true;
				}
				std::string fullTitle;
				if (prop->m_serie != "") {
					fullTitle += prop->m_serie + "-";
				}
				if (prop->m_saison != "") {
					fullTitle += "s" + prop->m_saison + "-";
				}
				if (prop->m_episode != "") {
					fullTitle += "e" + prop->m_episode + "-";
				}
				fullTitle += prop->m_title;
				APPL_DEBUG("info element : " << prop->m_id << " title: " << fullTitle);
				m_currentPayed = prop->m_id;
				signalSelect.emit(prop->m_id);
			} else if (m_listDisplay[findId]->m_propertyGroup != nullptr) {
				ememory::SharedPtr<appl::ElementPropertyGroup> prop = m_listDisplay[findId]->m_propertyGroup;
				if (prop == nullptr) {
					return true;
				}
				std::string newGroup = "";
				if (m_currentGroup == "type") {
					if (prop->m_filter == "film") {
						newGroup = "production-methode";
					} else if (prop->m_filter == "tv-show") {
						newGroup = "production-methode";
					}
				} else if (m_currentGroup == "production-methode") {
					if (etk::start_with(m_currentFilter, "'type' == 'tv-show'") == true) {
						newGroup = "series-name";
					}
				} else if (m_currentGroup == "series-name") {
					newGroup = "saison";
				} else if (m_currentGroup == "artist") {
					newGroup = "album";
				}
				if (m_currentFilter == "*") {
					searchElementsInternal("'" + m_currentGroup + "' == '" + prop->m_filter + "'", newGroup);
				} else {
					searchElementsInternal(m_currentFilter + " AND '" + m_currentGroup + "' == '" + prop->m_filter + "'", newGroup);
				}
			}
			return true;
		}
	}
	return false;
}

bool appl::widget::ListViewer::previous() {
	for (int64_t iii=0; iii<int64_t(m_listElement.size()); ++iii) {
		if (m_listElement[iii] == nullptr) {
			continue;
		}
		if (m_listElement[iii]->m_id == m_currentPayed) {
			// Start search ...
			--iii;
			while(iii>=0) {
				if (m_listElement[iii] == nullptr) {
					--iii;
					continue;
				}
				m_currentPayed = m_listElement[iii]->m_id;
				signalSelect.emit(m_currentPayed);
				return true;
			}
			return false;
		}
	}
	return false;
}

bool appl::widget::ListViewer::next() {
	for (size_t iii=0; iii<m_listElement.size(); ++iii) {
		if (m_listElement[iii] == nullptr) {
			continue;
		}
		if (m_listElement[iii]->m_id == m_currentPayed) {
			// Start search ...
			++iii;
			while(iii<m_listElement.size()) {
				if (m_listElement[iii] == nullptr) {
					++iii;
					continue;
				}
				m_currentPayed = m_listElement[iii]->m_id;
				signalSelect.emit(m_currentPayed);
				return true;
			}
			return false;
		}
	}
	return false;
}

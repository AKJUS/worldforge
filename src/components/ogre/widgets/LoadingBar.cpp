/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
Filename:    ExampleLoadingBar.h
Description: Defines an example loading progress bar which you can use during
startup, level changes etc to display loading progress.
IMPORTANT: Note that this progress bar relies on you having the OgreCore.zip
package already added to a resource group called 'Bootstrap' - this provides
the basic resources required for the progress bar and will be loaded automatically.
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "LoadingBar.h"

#include "services/EmberServices.h"
#include "services/wfut/WfutService.h"
#include "services/input/Input.h"
#include "framework/ShutdownException.h"
#include "framework/MainLoopController.h"

#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayManager.h>
#include <OgreMaterialManager.h>
#include <OgreFontManager.h>
#include <OgreRenderWindow.h>
#include <Ogre.h>

#include <utility>

using namespace Ogre;
namespace Ember {
namespace OgreView {
namespace Gui {

/** Defines an example loading progress bar which you can use during
	startup, level changes etc to display loading progress.
@remarks
	Basically you just need to create an instance of this class, call start()
	before loading and finish() afterwards. You may also need to stop areas of
	your scene rendering in between since this method will call
	RenderWindow::update() to update the display of the bar - we advise using
	SceneManager's 'special case render queues' for this, see
	SceneManager::addSpecialCaseRenderQueue for details.
@note
	This progress bar relies on you having the OgreCore.zip and EmberCore.zip package already
	added to a resource group called 'Bootstrap' - this provides the basic
	resources required for the progress bar and will be loaded automatically.
*/
LoadingBar::LoadingBar(Ogre::RenderWindow& window, MainLoopController& mainLoopController) :
		mProgress(0),
		mProgressBarMaxSize(0),
		mProgressBarMaxLeft(0),
		mWindow(window),
		mLoadOverlay(nullptr),
		mProgressBarScriptSize(0),
		mLoadingBarElement(nullptr),
		mLoadingDescriptionElement(nullptr),
		mLoadingCommentElement(nullptr),
		mVersionElement(nullptr),
		mMainLoopController(mainLoopController) {


}

LoadingBar::~LoadingBar() {
	try {
		if (mLoadOverlay) {
			//Make a copy since it will be invalidated by the calls to "destroy".
			auto elements = mLoadOverlay->get2DElements();
			for (auto* container : elements) {
				deleteOverlayContainerContents(*container);
				OverlayManager::getSingleton().destroyOverlayElement(container);
			}
			OverlayManager::getSingleton().destroy(mLoadOverlay);
		}
	} catch (const std::exception& ex) {
		S_LOG_WARNING("Error when destroying loading bar overlay." << ex);
	}

}

void LoadingBar::deleteOverlayContainerContents(Ogre::OverlayContainer& container) const {
	{
		auto I = container.getChildContainerIterator();
		while (I.hasMoreElements()) {
			deleteOverlayContainerContents(*I.getNext());
		}
	}
	auto I = container.getChildIterator();
	while (I.hasMoreElements()) {
		OverlayManager::getSingleton().destroyOverlayElement(I.getNext());
	}
}

void LoadingBar::start() {

	auto& resourceGrpMgr = Ogre::ResourceGroupManager::getSingleton();

	//Parse the materials used for the loading bar
	auto programPtr = resourceGrpMgr.openResource("common/ui/Overlay.program");
	Ogre::MaterialManager::getSingleton().parseScript(programPtr, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	auto materialPtr = resourceGrpMgr.openResource("common/ui/splash/EmberCore.material");
	Ogre::MaterialManager::getSingleton().parseScript(materialPtr, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	auto fontdefPtr = resourceGrpMgr.openResource("common/ui/splash/Ember.fontdef");
	Ogre::FontManager::getSingleton().parseScript(fontdefPtr, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	auto overlayPtr = resourceGrpMgr.openResource("common/ui/splash/EmberLoadingPanel.overlay");
	Ogre::OverlayManager::getSingleton().parseScript(overlayPtr, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);


	try {
		OverlayManager& omgr = OverlayManager::getSingleton();
		mLoadOverlay = omgr.getByName("EmberCore/LoadOverlay");
		if (!mLoadOverlay) {
			OGRE_EXCEPT(::Ogre::Exception::ERR_ITEM_NOT_FOUND, "Cannot find loading overlay", "LoadingBar::start");
		}
		mLoadOverlay->show();

		// Save links to the bar and to the loading text, for updates as we go
		mLoadingBarElement = omgr.getOverlayElement("EmberCore/LoadPanel/Bar/Progress");
		mLoadingCommentElement = omgr.getOverlayElement("EmberCore/LoadPanel/Comment");
		mLoadingDescriptionElement = omgr.getOverlayElement("EmberCore/LoadPanel/Description");
		mVersionElement = omgr.getOverlayElement("EmberCore/Splash/VersionInfo");

		//OverlayElement* barContainer = omgr.getOverlayElement("EmberCore/LoadPanel/Bar");
		mProgressBarMaxSize = mLoadingBarElement->getWidth();
		mProgressBarMaxLeft = mLoadingBarElement->getLeft();

		//mLoadingBarElement->setWidth(300);

	} catch (const std::exception& ex) {
		S_LOG_FAILURE("Error when creating loading bar." << ex);
	}


}

/** Hide the loading bar and stop listening.
*/
void LoadingBar::finish() {
	if (mLoadOverlay) {
		// hide loading screen
		mLoadOverlay->hide();
	}
}


void LoadingBar::addSection(LoadingBarSection* section) {
	mSections.push_back(section);
}


void LoadingBar::setProgress(float progress) {
	progress = std::max(0.f, std::min(1.0f, progress));
	if (mLoadingBarElement) {
		S_LOG_VERBOSE(progress);
		//make the black blocking block a little bit smaller and move it to the right
		mLoadingBarElement->setWidth(mProgressBarMaxSize * (1.f - progress));
		mLoadingBarElement->setLeft(mProgressBarMaxLeft + (mProgressBarMaxSize * progress));
		updateRender();
	}
	mProgress = progress;
}

void LoadingBar::increase(float amount) {
	setProgress(mProgress + amount);
}

void LoadingBar::setCaption(const std::string& caption) {
	if (mLoadingCommentElement) {
		mLoadingCommentElement->setCaption(caption);
		updateRender();
	}
}

void LoadingBar::setVersionText(const std::string& versionText) {
	if (mVersionElement) {
		mVersionElement->setCaption(versionText);
		updateRender();
	}
}

void LoadingBar::updateRender(bool forceUpdate) {
	static unsigned long oneFrame = 1000L / 60L;
	unsigned long millisecondsSinceLastFrame = mTimer.getMilliseconds();
	if (millisecondsSinceLastFrame > oneFrame || forceUpdate) {
		try {
			//There's a bug in Ogre 1.7.1 (at least) which makes the text of some elements not appear. By asking it to update the positions it seems to work.
			mVersionElement->_positionsOutOfDate();

			Input::getSingleton().processInput();
			if (mMainLoopController.shouldQuit() || mWindow.isClosed()) {
				throw ShutdownException("Aborting startup");
			}

			mWindow.update();
		} catch (const std::exception& ex) {
			S_LOG_FAILURE("Error when updating render for loading bar." << ex);
		}
		mTimer.reset();
	}
}

LoadingBarSection::LoadingBarSection(LoadingBar& loadingBar, float size, std::string name)
		: mSize(size), mLoadingBar(loadingBar), mAccumulatedSize(0), mName(std::move(name)), mActive(false) {
}

const std::string& LoadingBarSection::getName() const {
	return mName;
}

void LoadingBarSection::setCaption(const std::string& caption) {
	mLoadingBar.setCaption(caption);
}

float LoadingBarSection::getSize() const {
	return mSize;
}

void LoadingBarSection::tick(float tickSize) {
	if (mAccumulatedSize < 1.0) {
		mLoadingBar.increase(mSize * tickSize);
		mAccumulatedSize += tickSize;
	}
}

void LoadingBarSection::setProgress(float progress) {
	if (mAccumulatedSize < 1.0 && mAccumulatedSize < progress) {
		mLoadingBar.increase(mSize * (mAccumulatedSize - progress));
		mAccumulatedSize = progress;
		mLoadingBar.updateRender();
	}
}

void LoadingBarSection::deactivate() {
	mActive = false;
}


ResourceGroupLoadingBarSection::ResourceGroupLoadingBarSection(LoadingBarSection& section,
															   unsigned short numGroupsInit,
															   unsigned short numGroupsLoad,
															   Ogre::Real initProportion)
		: mInitProportion(initProportion), mNumGroupsInit(numGroupsInit), mNumGroupsLoad(numGroupsLoad), mSection(section), mProgressBarInc(0), mCompletedSections(0) {
	// self is listener
	ResourceGroupManager::getSingleton().addResourceGroupListener(this);
}

ResourceGroupLoadingBarSection::~ResourceGroupLoadingBarSection() {
	ResourceGroupManager::getSingleton().removeResourceGroupListener(this);
}

// ResourceGroupListener callbacks
void ResourceGroupLoadingBarSection::resourceGroupScriptingStarted(const String& groupName, size_t scriptCount) {
	if (mNumGroupsInit == 0) {
		return; //avoid divide-by-zero
	}
	if (mNumGroupsLoad != 0) {
		mProgressBarInc = mInitProportion / (float) mNumGroupsInit;
	} else {
		mProgressBarInc = 1.0f / (float) mNumGroupsInit;
	}

	if (scriptCount == 0) {
		//no scripts will be loaded, so we'll have to conclude this group here and now
		mSection.tick(mProgressBarInc);
	}

	mProgressBarInc /= (Real) scriptCount;
	mSection.setCaption("Parsing scripts...");
}

void ResourceGroupLoadingBarSection::resourceGroupScriptingEnded(const Ogre::String& groupName) {
	mCompletedSections++;
	mSection.setProgress((float) mCompletedSections / (float) (mNumGroupsInit + mNumGroupsLoad));
}

void ResourceGroupLoadingBarSection::scriptParseStarted(const String& scriptName, bool& skipThisScript) {
	mSection.setCaption(scriptName);
}

void ResourceGroupLoadingBarSection::scriptParseEnded(const Ogre::String& scriptName, bool) {
	//make the black blocking block a little bit smaller and move it to the right
	mSection.tick(mProgressBarInc);
}

void ResourceGroupLoadingBarSection::resourceGroupLoadStarted(const String& groupName, size_t resourceCount) {
	if (mNumGroupsLoad == 0) {
		return; //avoid divide-by-zero
	}
	if (mNumGroupsInit) {
		mProgressBarInc = (1.0f - mInitProportion) / (float) mNumGroupsLoad;
	} else {
		mProgressBarInc = 1.0f / (float) mNumGroupsLoad;
	}

	if (resourceCount == 0) {
		//no resources will be loaded, so we'll have to conclude this group here and now
		mSection.tick(mProgressBarInc);
	}

	mProgressBarInc /= (Real) resourceCount;
	mSection.setCaption("Loading resources...");
}

void ResourceGroupLoadingBarSection::resourceLoadStarted(const ResourcePtr& resource) {
	mSection.setCaption(resource->getName());
}

void ResourceGroupLoadingBarSection::resourceLoadEnded() {
	mSection.tick(mProgressBarInc);
}

void ResourceGroupLoadingBarSection::resourceGroupLoadEnded(const String& groupName) {
	mCompletedSections++;
	mSection.setProgress((float) mCompletedSections / (float) (mNumGroupsInit + mNumGroupsLoad));
}


WfutLoadingBarSection::WfutLoadingBarSection(LoadingBarSection& section)
		: mSection(section), mNumberOfFilesToUpdate(0), mDownloadedSoFar(0) {
	WfutService& wfutSrv = EmberServices::getSingleton().getWfutService();
	wfutSrv.DownloadComplete.connect(sigc::mem_fun(*this, &WfutLoadingBarSection::wfutService_DownloadComplete));
	wfutSrv.DownloadFailed.connect(sigc::mem_fun(*this, &WfutLoadingBarSection::wfutService_DownloadFailed));
	wfutSrv.AllDownloadsComplete.connect(sigc::mem_fun(*this, &WfutLoadingBarSection::wfutService_AllDownloadsComplete));
	wfutSrv.DownloadingServerList.connect(sigc::mem_fun(*this, &WfutLoadingBarSection::wfutService_DownloadingServerList));
	wfutSrv.UpdatesCalculated.connect(sigc::mem_fun(*this, &WfutLoadingBarSection::wfutService_UpdatesCalculated));
}

void WfutLoadingBarSection::wfutService_DownloadComplete(const std::string& url, const std::string& filename) {
	mDownloadedSoFar++;
	std::stringstream ss;
	ss << "Downloaded " << filename << " (" << mDownloadedSoFar << " of " << mNumberOfFilesToUpdate << ")";
	mSection.setCaption(ss.str());
	if (mNumberOfFilesToUpdate) {
		mSection.tick(1.0f / (float) mNumberOfFilesToUpdate);
	}
}

void WfutLoadingBarSection::wfutService_DownloadFailed(const std::string& url, const std::string& filename, const std::string& reason) {
	mDownloadedSoFar++;
	std::stringstream ss;
	ss << "Failed to download " << filename << " (" << mDownloadedSoFar << " of " << mNumberOfFilesToUpdate << ")";
	mSection.setCaption(ss.str());
	if (mNumberOfFilesToUpdate) {
		mSection.tick(1.0f / (float) mNumberOfFilesToUpdate);
	}
}

void WfutLoadingBarSection::wfutService_AllDownloadsComplete() {
}

void WfutLoadingBarSection::wfutService_DownloadingServerList(const std::string& url) {
	mSection.setCaption("Getting server list from " + url);
}

void WfutLoadingBarSection::wfutService_UpdatesCalculated(unsigned int numberOfFilesToUpdate) {
	mNumberOfFilesToUpdate = numberOfFilesToUpdate;
}

}
}
}

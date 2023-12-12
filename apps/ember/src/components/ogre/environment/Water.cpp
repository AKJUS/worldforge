//
// C++ Implementation: Water
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2004
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.//
//
#include "Water.h"
#include <OgreSceneManager.h>
#include <OgreRenderTargetListener.h>
#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreMeshManager.h>
#include <OgreGpuProgramManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreCamera.h>
#include <OgreRenderTexture.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgreViewport.h>
#include <OgreTextureManager.h>

using namespace Ogre;




namespace Ember::OgreView::Environment {

class RefractionTextureListener : public RenderTargetListener {
	Entity* pPlaneEnt{};
	std::vector<Entity*> aboveWaterEnts;

public:

	RefractionTextureListener() = default;

	void setPlaneEntity(Entity* plane) {
		pPlaneEnt = plane;
	}


	void preRenderTargetUpdate(const RenderTargetEvent& evt) override {
		// Hide plane and objects above the water
		pPlaneEnt->setVisible(false);
		std::vector<Entity*>::iterator i, iend;
		iend = aboveWaterEnts.end();
		for (i = aboveWaterEnts.begin(); i != iend; ++i) {
			(*i)->setVisible(false);
		}

	}

	void postRenderTargetUpdate(const RenderTargetEvent& evt) override {
		// Show plane and objects above the water
		pPlaneEnt->setVisible(true);
		std::vector<Entity*>::iterator i, iend;
		iend = aboveWaterEnts.end();
		for (i = aboveWaterEnts.begin(); i != iend; ++i) {
			(*i)->setVisible(true);
		}
	}

};

class ReflectionTextureListener : public RenderTargetListener {
	Plane reflectionPlane;
	Entity* pPlaneEnt{};
	std::vector<Entity*> belowWaterEnts;
	Ogre::Camera* theCam{};

public:

	ReflectionTextureListener() = default;

	void setPlaneEntity(Entity* plane) {
		pPlaneEnt = plane;
	}

	void setReflectionPlane(Plane& aPlane) {
		reflectionPlane = aPlane;
	}

	void setCamera(Ogre::Camera* aCamera) {
		theCam = aCamera;
	}


	void preRenderTargetUpdate(const RenderTargetEvent& evt) override {
		// Hide plane and objects below the water
		pPlaneEnt->setVisible(false);
		std::vector<Entity*>::iterator i, iend;
		iend = belowWaterEnts.end();
		for (i = belowWaterEnts.begin(); i != iend; ++i) {
			(*i)->setVisible(false);
		}
		theCam->enableReflection(reflectionPlane);

	}

	void postRenderTargetUpdate(const RenderTargetEvent& evt) override {
		// Show plane and objects below the water
		pPlaneEnt->setVisible(true);
		std::vector<Entity*>::iterator i, iend;
		iend = belowWaterEnts.end();
		for (i = belowWaterEnts.begin(); i != iend; ++i) {
			(*i)->setVisible(true);
		}
		theCam->disableReflection();
	}

};


Water::Water(Ogre::Camera& camera, Ogre::SceneManager& sceneMgr)
		: mCamera(camera),
		  mSceneMgr(sceneMgr),
		  mRefractionListener(nullptr),
		  mReflectionListener(nullptr),
		  mWaterNode(nullptr),
		  mWaterEntity(nullptr) {
}

bool Water::isSupported() const {
	// Check prerequisites first
	const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
	if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !(caps->hasCapability(RSC_FRAGMENT_PROGRAM))) {
		return false;
	} else {
		if (!GpuProgramManager::isSyntaxSupported("arbfp1") &&
			!GpuProgramManager::isSyntaxSupported("ps_2_0") &&
			!GpuProgramManager::isSyntaxSupported("ps_1_4")
				) {
			return false;
		}
	}

	return true;
}


/**
	* @brief Initializes the water. You must call this in order for the water to show up.
	* @return True if the water technique could be setup, else false.
	*/
bool Water::initialize() {
	try {
		Ogre::Plane waterPlane(Ogre::Vector3::UNIT_Y,
		0);


		// create a water plane/scene node
/*		waterPlane.normal = ;
		waterPlane.d = 0; */
		Ogre::MeshManager::getSingleton().createPlane(
				"WaterPlane",
				"environment",
				waterPlane,
				10000, 10000,
				5, 5,
				true, 1,
				1000, 1000,
				Ogre::Vector3::UNIT_Z
		);

		mWaterNode = mSceneMgr.getRootSceneNode()->createChildSceneNode("water");

		mRefractionListener = std::make_unique<RefractionTextureListener>();
		mReflectionListener = std::make_unique<ReflectionTextureListener>();

		Ogre::TexturePtr texture = TextureManager::getSingleton().createManual("Refraction", "General", TEX_TYPE_2D, 512, 512, 1, PF_A8R8G8B8, TU_RENDERTARGET);
		RenderTexture* rttTex = texture->getBuffer()->getRenderTarget();
/*		RenderTexture* rttTex = EmberOgre::getSingleton().getOgreRoot()->getRenderSystem()->createRenderTexture( "Refraction", 512, 512 );*/

		{
			Viewport* v = rttTex->addViewport(&mCamera);
			Ogre::MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/FresnelReflectionRefraction");
			if (mat) {
				mat->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName("Refraction");
				v->setOverlaysEnabled(false);
				rttTex->addListener(mRefractionListener.get());
			}
		}


		texture = TextureManager::getSingleton().createManual("Reflection", "General", TEX_TYPE_2D, 512, 512, 0, PF_A8R8G8B8, TU_RENDERTARGET);
		rttTex = texture->getBuffer()->getRenderTarget();
		{
			Viewport* v = rttTex->addViewport(&mCamera);
			Ogre::MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/FresnelReflectionRefraction");
			if (mat) {
				mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName("Reflection");
				v->setOverlaysEnabled(false);
				rttTex->addListener(mReflectionListener.get());
			}
		}

		// Define a floor plane mesh
/*			reflectionPlane.normal = Vector3::UNIT_Y;
			reflectionPlane.d = 0;*/
/*			MeshManager::getSingleton().createPlane("ReflectPlane",reflectionPlane,
				1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);*/


		mWaterEntity = mSceneMgr.createEntity("plane", "WaterPlane");
		mWaterEntity->setMaterialName("Examples/FresnelReflectionRefraction");
		mRefractionListener->setPlaneEntity(mWaterEntity);
		mReflectionListener->setPlaneEntity(mWaterEntity);
		mReflectionListener->setReflectionPlane(mReflectionPlane);
		mReflectionListener->setCamera(&mCamera);
		mWaterNode->attachObject(mWaterEntity);
		return true;
	} catch (const std::exception& ex) {
		logger->error("Error when creating water: {}", ex.what());
		return false;
	}

}


Water::~Water() {
	if (mWaterNode) {
		mWaterNode->detachAllObjects();
		mSceneMgr.destroySceneNode(mWaterNode);
	}
	if (mWaterEntity) {
		mSceneMgr.destroyEntity(mWaterEntity);
	}
}


}




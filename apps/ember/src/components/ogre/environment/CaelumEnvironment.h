//
// C++ Interface: CaelumEnvironment
//
// Description: 
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2006
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
#ifndef EMBEROGRE_ENVIRONMENTCAELUMENVIRONMENT_H
#define EMBEROGRE_ENVIRONMENTCAELUMENVIRONMENT_H

#include "components/ogre/EmberOgrePrerequisites.h"
#include "IEnvironmentProvider.h"
#include "framework/ConsoleCommandWrapper.h"

#include <sigc++/trackable.h>

namespace Caelum {
class CaelumSystem;

class SkyDome;

class BaseSkyLight;
}

namespace Eris {
class Calendar;
}




namespace Ember::OgreView::Environment {

class CaelumSky;

class CaelumSun;

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 */
class CaelumEnvironment : public IEnvironmentProvider, public ConsoleObject, public virtual sigc::trackable {
public:
	CaelumEnvironment(Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window, Ogre::Camera& camera, const Eris::Calendar& calendar);

	~CaelumEnvironment() override;

	void createFirmament() override;

	void destroyFirmament() override;

	void setWaterEnabled(bool enabled) override;

	ISun* getSun() override;

	ISky* getSky() override;

	IFog* getFog() override;

	IWater* getWater() override;

	Caelum::CaelumSystem* getCaelumSystem() const;

	const ConsoleCommandWrapper SetCaelumTime;

	void setTime(int hour, int minute, int second) override;

	void setTime(int seconds) override;

	void setTimeMultiplier(float multiplier) override;

	float getTimeMultiplier() const override;

	/**
	 * @brief Sets the position of the world.
	 * @param longitudeDegrees The longitude, as degrees.
	 * @param latitudeDegrees The latitude, as degrees.
	 */
	void setWorldPosition(float longitudeDegrees, float latitudeDegrees) override;

	void runCommand(const std::string& command, const std::string& args) override;

private:

	void destroyCaelumFirmament();

	/**
	 * @brief Creates and initializes the Caelum system.
	 * @param sceneMgr 
	 * @param window 
	 * @param camera 
	 */
	void setupCaelum(Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window, Ogre::Camera& camera);

	/**
	 * @brief Creates a water plane.
	 */
	void setupWater();

	/**
	 * @brief Called when the calendar is updated.
	 */
	void Calendar_Updated();

	// Caelum system
	std::unique_ptr<Caelum::CaelumSystem> mCaelumSystem;

	Ogre::SceneManager* mSceneMgr;
	Ogre::RenderWindow* mWindow;
	Ogre::Camera& mCamera;

	/**
	 * @brief Holds the main calendar instance for this world.
	 */
	const Eris::Calendar& mCalendar;

	std::unique_ptr<CaelumSky> mSky;
	std::unique_ptr<CaelumSun> mSun;
	std::unique_ptr<IWater> mWater;

};

inline Caelum::CaelumSystem* CaelumEnvironment::getCaelumSystem() const {
	return mCaelumSystem.get();
}

class CaelumEnvironmentComponent {
protected:
	explicit CaelumEnvironmentComponent(CaelumEnvironment& environment) :
			mEnvironment(environment), mCaelumSystem(environment.getCaelumSystem()) {
	}

	CaelumEnvironment& mEnvironment;

	/// Caelum system
	Caelum::CaelumSystem* mCaelumSystem;

};

}





#endif

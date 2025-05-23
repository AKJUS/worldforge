/*
 Copyright (C) 2011 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef IENVIRONMENTPROVIDER_H_
#define IENVIRONMENTPROVIDER_H_

#include "components/ogre/ILightning.h"
#include "components/ogre/OgreIncludes.h"


namespace Ember::OgreView::Environment {

struct ISun : public ILightning {
	~ISun() override = default;

	virtual void setAmbientLight(const Ogre::ColourValue& colour) = 0;

	virtual Ogre::Vector3 getSunDirection() const = 0;

};

struct ISky {
};

struct IFog {
	virtual ~IFog() = default;

	virtual void setDensity(float density) = 0;

	virtual float getDensity() const = 0;
};

/**
 @brief Interface for all water techniques. Call isSupported() first to see if the water can be created, and then initialize() to set it all up.
 @author Erik Ogenvik <erik@worldforge.org>
 */
struct IWater {
	virtual ~IWater() = default;

	/**
	 * @brief Performs checks to see whether this technique is supported on the current hardware.
	 * @return True if the water technique is supported.
	 */
	virtual bool isSupported() const = 0;

	/**
	 * @brief Initializes the water. You must call this in order for the water to show up.
	 * @return True if the water technique could be setup, else false.
	 */
	virtual bool initialize() = 0;

	/**
	 * @brief Sets the level of the water.
	 * @param height The height of the water level, in world units.
	 */
	virtual void setLevel(float height) = 0;

	/**
	 * @brief Attaches a user object, if possible, to the water.
	 * @param The user object to attach.
	 * @note If the object could be attached, ownership is transferred to the water entity. If however it couldn't be attached, it's responsibility of the calling method to make sure that the object is destroyed.
	 * @return True if the user object was successfully attached. This means that ownership transfers to the water instance.
	 */
	virtual bool setUserAny(const Ogre::Any& anything) = 0;
};

/**
 * @brief An interface for environment providers.
 *
 * An implementation of this is expected to be able to supply the instances which make up the environment.
 */
struct IEnvironmentProvider {

	virtual ~IEnvironmentProvider() = default;

	/**
	 * @brief Creates the firmament (sky, sun, moon, starfield etc.).
	 */
	virtual void createFirmament() = 0;

	/**
	 * @brief Destroys the firmament.
	 */
	virtual void destroyFirmament() = 0;

	virtual ISun* getSun() = 0;

	virtual ISky* getSky() = 0;

	virtual IFog* getFog() = 0;

	virtual IWater* getWater() = 0;

	/**
	 * @brief Sets whether the water (ocean) is enabled or not.
	 * @param enabled True if the water should enabled.
	 */
	virtual void setWaterEnabled(bool enabled) = 0;

	/**
	 * @brief Sets the current time.
	 * @param hour The hour.
	 * @param minute The minute.
	 * @param second The second.
	 */
	virtual void setTime(int hour, int minute, int second) = 0;

	/**
	 * @brief Sets the current time.
	 * @param seconds Seconds since midnight.
	 */
	virtual void setTime(int seconds) = 0;

	/**
	 * @brief Sets the time multiplier, i.e. how much the time of the environment will progress per real time second.
	 * @param multiplier The time multiplier.
	 */
	virtual void setTimeMultiplier(float multiplier) = 0;

	/**
	 * @brief Gets the time multiplier, i.e. how much the time of the environment will progress per real time second.
	 * @return The time multiplier.
	 */
	virtual float getTimeMultiplier() const = 0;

	/**
	 * @brief Sets the position of the world.
	 * @param longitudeDegrees The longitude, as degrees.
	 * @param latitudeDegrees The latitude, as degrees.
	 */
	virtual void setWorldPosition(float longitudeDegrees, float latitudeDegrees) = 0;

};

}



#endif /* IENVIRONMENTPROVIDER_H_ */

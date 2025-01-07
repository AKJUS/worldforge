//
// C++ Interface: SoundSource
//
// Description: 
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2008
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

#ifndef EMBERSOUNDSOURCE_H
#define EMBERSOUNDSOURCE_H

#include <wfmath/point.h>

#include "al.h"

namespace Ember {

/**
 * @brief Represents a sound source in the 3d world.
 * An instance of this class mainly envelopes an OpenAL soundsource, allocating it when it's created and removing it at destruction time.
 * @author Erik Ogenvik <erik@ogenvik.org>
 */
class SoundSource {
public:

	/**
	* @brief Ctor.
	* At construction time the sound source will be created in OpenAL and initialized to default values.
	*/
	SoundSource();

	SoundSource(SoundSource&& rhs) = default;

	/**
	 * @brief Dtor.
	 * When an instance is destroyed the OpenAL sound source will be released as well.
	 */
	virtual ~SoundSource();

	/**
	 * @brief Sets the position of the sound source.
	 * @param position The position, in world units.
	 */
	void setPosition(const WFMath::Point<3>& position) const;

	/**
	 * @brief Sets the velocity of the sound source.
	 * @param velocity The velocity, in world units.
	 */
	void setVelocity(const WFMath::Vector<3>& velocity) const;

	/**
	 * Sets gain (volume)
	 * @param gain
	 */
	void setGain(float gain) const;

	void setRolloff(float rolloff) const;

	void setReference(float reference) const;

	/**
	* @brief Return openAl source within this sample
	* @return The identifier of the source
	*/
	ALuint getALSource() const;

protected:

	/**
	 * @brief The OpenAL source which this class represents.
	 */
	ALuint mALSource;

};

inline ALuint SoundSource::getALSource() const {
	return mALSource;
}




}

#endif

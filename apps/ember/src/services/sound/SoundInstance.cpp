//
// C++ Implementation: SoundInstance
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
#include "SoundInstance.h"
#include "SoundSample.h"
#include "SoundSource.h"

namespace Ember {

SoundInstance::SoundInstance()
		: mSource(std::make_unique<SoundSource>()),
		  mMotionProvider(nullptr),
		  mPreviousState(0) {
}


SoundInstance::~SoundInstance() {
	stop();
}

SoundSource& SoundInstance::getSource() {
	return *mSource;
}

bool SoundInstance::play() {
	alGetError();
	alSourcePlay(mSource->getALSource());
	mPreviousState = AL_PLAYING;
	return SoundGeneral::checkAlError("Playing sound instance.");
}

bool SoundInstance::stop() {
	alGetError();
	alSourceStop(mSource->getALSource());
	return SoundGeneral::checkAlError("Stopping sound instance.");
}

bool SoundInstance::pause() {
	alGetError();
	alSourcePause(mSource->getALSource());
	return SoundGeneral::checkAlError("Pausing sound instance.");
}

void SoundInstance::update() {
	if (mMotionProvider) {
		mMotionProvider->update(*mSource);
	}
	if (mBinding) {
		mBinding->update();
	}
	if (!getIsLooping()) {
		if (mPreviousState == AL_PLAYING) {
			ALint alNewState;
			alGetSourcei(mSource->getALSource(), AL_SOURCE_STATE, &alNewState);
			SoundGeneral::checkAlError("Checking source state.");
			if (alNewState == AL_STOPPED) {
				EventPlayComplete.emit();
				mPreviousState = alNewState;
			}
		}
	}
}


void SoundInstance::setIsLooping(bool isLooping) {
	alSourcei(mSource->getALSource(), AL_LOOPING, isLooping ? AL_TRUE : AL_FALSE);
	SoundGeneral::checkAlError("Setting looping status.");
}

bool SoundInstance::getIsLooping() const {
	ALint alValue;
	alGetSourcei(mSource->getALSource(), AL_LOOPING, &alValue);
	SoundGeneral::checkAlError("Checking looping status.");
	return alValue == AL_TRUE;
}

void SoundInstance::setMaxDistance(float maxDistance) {
	alSourcef(mSource->getALSource(), AL_MAX_DISTANCE, maxDistance);
	SoundGeneral::checkAlError("Setting max distance.");
}

float SoundInstance::getMaxDistance() const {
	ALfloat alValue;
	alGetSourcef(mSource->getALSource(), AL_MAX_DISTANCE, &alValue);
	SoundGeneral::checkAlError("Checking max distance.");
	return alValue;
}


}

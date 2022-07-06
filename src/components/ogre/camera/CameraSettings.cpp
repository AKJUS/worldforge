/*
 Copyright (C) 2011 Erik Ogenvik <erik@ogenvik.org>

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "CameraSettings.h"

namespace Ember
{
namespace OgreView
{
namespace Camera
{
CameraSettings::CameraSettings() : mDegreesPerMouseUnit(180), mInvertCamera(false)
{
	registerConfigListenerWithDefaults("input", "degreespermouseunit", sigc::mem_fun(*this, &CameraSettings::Config_DegreesPerMouseUnit), mDegreesPerMouseUnit);
	registerConfigListenerWithDefaults("input", "invertcamera", sigc::mem_fun(*this, &CameraSettings::Config_InvertCamera), mInvertCamera);
}

CameraSettings::~CameraSettings() = default;

void CameraSettings::Config_DegreesPerMouseUnit(const std::string& /*section*/, const std::string& /*key*/, varconf::Variable& variable)
{
	if (variable.is_double()) {
		mDegreesPerMouseUnit = (float)static_cast<double>(variable);
	}
}

void CameraSettings::Config_InvertCamera(const std::string& /*section*/, const std::string& /*key*/, varconf::Variable& variable)
{
	if (variable.is_bool()) {
		mInvertCamera = static_cast<bool>(variable);
	}
}

}
}
}

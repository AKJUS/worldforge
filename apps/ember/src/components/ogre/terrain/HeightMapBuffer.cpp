/*
 Copyright (C) 2009 Erik Ogenvik <erik@ogenvik.org>

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

#include "HeightMapBuffer.h"
#include "HeightMapBufferProvider.h"
#include "Buffer.h"


namespace Ember::OgreView::Terrain {

HeightMapBuffer::HeightMapBuffer(HeightMapBufferProvider& provider, std::unique_ptr<BufferType> buffer) :
		mProvider(provider),
		mBuffer(std::move(buffer)) {
}

HeightMapBuffer::~HeightMapBuffer() {
	mProvider.checkin(std::move(this->mBuffer));
}

HeightMapBuffer::BufferType& HeightMapBuffer::getBuffer() {
	return *mBuffer;
}

unsigned int HeightMapBuffer::getResolution() const {
	return mBuffer->getResolution();
}

}




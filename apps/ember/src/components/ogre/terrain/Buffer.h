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


#ifndef EMBEROGRETERRAINBUFFER_H_
#define EMBEROGRETERRAINBUFFER_H_

#include <string>
#include <vector>


namespace Ember::OgreView::Terrain {

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 * @brief A simple image bitmap buffer, square in size.
 */
template<typename DataType>
class Buffer {
public:

	/**
	 * @brief Ctor.
	 * @param resolution The size, in pixels, of one side of the square.
	 * @param channels The number of channels.
	 * @note By using this constructor a data structure owned by this instance will automatically be created.
	 */
	Buffer(unsigned int width, unsigned int channels);

	/**
	 * @brief Dtor.
	 */
	~Buffer();

	/**
	 * @brief Accessor for the underlying bitmap data.
	 * @returns The underlying bitmap data.
	 */
	DataType* getData();

	/**
	 * @brief Accessor for the underlying bitmap data.
	 * @returns The underlying bitmap data.
	 */
	const DataType* getData() const;

	/**
	 * @brief Gets the value at the specified position.
	 * @note No bounds checking will be done, so be sure that the positions are valid.
	 * @param x X position.
	 * @param y Y position.
	 * @param channel The channel to .
	 */
	DataType getValueAt(unsigned int x, unsigned int y, unsigned int channel);

	/**
	 * @brief Accessor for the number of channels.
	 * @returns The number of channels.
	 */
	unsigned int getChannels() const;

	/**
	 * @brief Gets the size of the underlying data.
	 * @returns The size of the underlying data.
	 */
	size_t getSize() const;

	/**
	 * @brief Resets the buffer, setting all values to 0.
	 */
	void reset();

	/**
	 * @brief Gets the size in pixels of one side of the bitmap square.
	 * @returns The size in pixels of one side of the bitmap square.
	 */
	unsigned int getResolution() const;

protected:

	/**
	 * @brief The resolution of the bitmap, i.e. the size in pixels of one side of the bitmap square.
	 */
	const unsigned int mResolution;

	/**
	 * @brief The number of channels in the bitmap image.
	 */
	const unsigned int mChannels;

	/**
	 * @brief The underlying data which makes up the bitmap.
	 * This can be owned by this instance and thus freed when the instance is destroyed.
	 */
	std::vector<DataType> mData;

};

template<typename DataType>
Buffer<DataType>::Buffer(unsigned int width, unsigned int channels) :
		mResolution(width),
		mChannels(channels),
		mData(width * width * channels) {
}

template<typename DataType>
Buffer<DataType>::~Buffer() = default;

template<typename DataType>
DataType* Buffer<DataType>::getData() {
	return mData.data();
}

template<typename DataType>
const DataType* Buffer<DataType>::getData() const {
	return mData.data();
}

template<typename DataType>
DataType Buffer<DataType>::getValueAt(unsigned int x, unsigned int y, unsigned int channel) {
	return mData[(y * mResolution * mChannels) + (x * mChannels) + (channel - 1)];
}

template<typename DataType>
unsigned int Buffer<DataType>::getChannels() const {
	return mChannels;
}

template<typename DataType>
size_t Buffer<DataType>::getSize() const {
	return mData.size();
}

template<typename DataType>
unsigned int Buffer<DataType>::getResolution() const {
	return mResolution;
}

}


#endif /* EMBEROGRETERRAINBUFFER_H_ */

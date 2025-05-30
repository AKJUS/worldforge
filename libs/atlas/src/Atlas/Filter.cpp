// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2000 Michael Day

// $Id$

#include <Atlas/Filter.h>
#include <cstring>

namespace Atlas {

Filter::Filter(std::unique_ptr<Filter> next)
		: m_next(std::move(next)) {
}

Filter::~Filter() = default;

filterbuf::~filterbuf() {
	sync();
}

int_type filterbuf::overflow(int_type c) {
	if (c != traits_type::eof()) {
		*pptr() = (char) c;
		pbump(1);
	}
	if (flushOutBuffer() == traits_type::eof()) {
		return traits_type::eof();
	}
	return c;
}

int_type filterbuf::underflow() {
	if (gptr() < egptr()) return *gptr();

	auto numPutback = gptr() - eback();

	if (numPutback > m_inPutback) numPutback = m_inPutback;

	std::memcpy(m_outBuffer.data() + (m_inPutback - numPutback),
				gptr() - numPutback,
				(unsigned long) numPutback);

	//     FIXME
	// Here we need to actually
	//  * get data from m_streamBuffer
	//  * encode it with m_filter
	//  * put _that_ into the buffer
	//
	// Currently it just fetches it and places it straight in the
	// buffer.
	// The problem is the limited size of the buffer with the
	// Filter::decode operation not having any kind of size
	// limitation.
	auto num = m_streamBuffer.sgetn(m_inBuffer.data() + m_inPutback,
							   m_inBuffer.size() - m_inPutback);
	if (num <= 0) return traits_type::eof();

	setg(m_inBuffer.data() + (m_inPutback - numPutback),
		 m_inBuffer.data() + m_inPutback,
		 m_inBuffer.data() + m_inPutback + num);

	return *gptr();
}

int filterbuf::sync() {
	if (flushOutBuffer() == traits_type::eof()) return -1;
	return 0;
}

} // namespace Atlas

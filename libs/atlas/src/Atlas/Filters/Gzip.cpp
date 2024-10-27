// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2000 Dmitry Derevyanko

// $Id$

#if defined(HAVE_ZLIB_H) && defined(HAVE_LIBZ)
#define ZLIB_CONST

#include <Atlas/Filters/Gzip.h>

#ifndef ASSERT

#include <cassert>

#define ASSERT(exp) assert(exp)
#endif

using Atlas::Filters::Gzip;

const int DEFAULT_LEVEL = 6;

void Gzip::begin() {
	incoming.next_in = Z_NULL;
	incoming.avail_in = 0;
	incoming.zalloc = Z_NULL;
	incoming.zfree = Z_NULL;

	outgoing.zalloc = Z_NULL;
	outgoing.zfree = Z_NULL;

	inflateInit(&incoming);
	deflateInit(&outgoing, DEFAULT_LEVEL);
}

void Gzip::end() {
	inflateEnd(&incoming);
	deflateEnd(&outgoing);
}

std::string Gzip::encode(const std::string& data) {
	std::string out_string;
	int status;

	buf[0] = 0;

//Handle early zlib versions
#if ZLIB_VERNUM < 0x1252
	outgoing.next_in = (unsigned char *)data.data();
#else
	outgoing.next_in = (z_const Bytef*) data.data();
#endif
	outgoing.avail_in = (uInt) data.size();

	do {
		outgoing.next_out = buf;
		outgoing.avail_out = sizeof(buf);

		status = deflate(&outgoing, Z_SYNC_FLUSH);

		ASSERT(status == Z_OK);

		if (status >= 0) {
			out_string.append((char*) buf, sizeof(buf) - outgoing.avail_out);
		}
	} while (outgoing.avail_out == 0);

	return out_string;
}

std::string Gzip::decode(const std::string& data) {
	std::string out_string;

	buf[0] = 0;

//Handle early zlib versions
#if ZLIB_VERNUM < 0x1252
	incoming.next_in = (unsigned char *)data.data();
#else
	incoming.next_in = (z_const Bytef*) data.data();
#endif
	incoming.avail_in = (uInt) data.size();

	do {
		incoming.next_out = buf;
		incoming.avail_out = sizeof(buf);

		int status = inflate(&incoming, Z_SYNC_FLUSH);

		ASSERT(status == Z_OK);

		if (status >= 0) {
			out_string.append((char*) buf, sizeof(buf) - incoming.avail_out);
		}
	} while (incoming.avail_out == 0);

	return out_string;
}

#endif // HAVE_ZLIB_H && HAVE_LIBZ

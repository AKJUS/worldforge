/*
 Copyright (C) 2023 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "SquallHandler.h"
#include "common/log.h"
#include "bytesize/bytesize.hh"

#include <filesystem>
#include <fstream>

HttpHandling::HttpHandler buildSquallHandler(std::filesystem::path repositoryDataPath) {
	return [repositoryDataPath = std::move(repositoryDataPath)](HttpHandleContext context) ->boost::asio::awaitable< HttpHandling::HandleResult> {
		if (context.path.starts_with("/squall/")) {
			auto squallPathSegment = context.path.substr(8);
			auto squallPath = repositoryDataPath / squallPathSegment;
			auto absolutePath = std::filesystem::absolute(squallPath);
			auto relative = std::filesystem::relative(absolutePath, repositoryDataPath);
			if (relative.empty()) {
				spdlog::warn("Requested path {} is not relative to squall root path. Someone is trying to compromise the system.", context.path);
				HttpHandling::reportBadRequest(context.io, 401, "Invalid request");
			} else {
				if (!std::filesystem::exists(absolutePath)) {
					spdlog::debug("Squall path '{}'not found.", context.path);
					HttpHandling::reportBadRequest(context.io, 404, "Not Found");
				} else {
					if (std::ifstream is{absolutePath, std::ios::binary | std::ios::ate}) {
						auto size = is.tellg();
						spdlog::debug("Serving up '{}', with size of {}.", absolutePath.generic_string(), bytesize::bytesize(size));
						is.seekg(0, std::ios::beg);
						HttpHandling::sendHeaders(context.io, 200, "application/octet-stream", "OK", {fmt::format("Content-Length: {}", std::to_string(size))});

						//Blocking write here, should be improved
						std::array<char, 2028> buffer{};
						while (is) {
							is.read(buffer.data(), buffer.size());
							auto readSize = is.gcount();
							context.io.write(buffer.data(), readSize);
						}
					}
				}
			}
			co_return HttpHandling::HandleResult::Handled;
		} else {
			co_return HttpHandling::HandleResult::Ignored;
		}
	};
}

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

#include "AssetsHandler.h"
#include "SquallAssetsGenerator.h"
#include "common/globals.h"
#include <spdlog/spdlog.h>

#include <utility>
#include "common/SynchedState_impl.h"

AssetsHandler::AssetsHandler(std::filesystem::path squallRepositoryPath)
		: mSquallRepositoryPath(std::move(squallRepositoryPath)) {

}

std::string AssetsHandler::resolveAssetsUrl() const {
	auto signature = mState.withStateConst<std::optional<Squall::Signature>>([](auto state) { return state->mSquallSignature; });

	if (signature) {
		//By omitting host we're telling the client to use the same host as the current connection.
//    return std::string("http://:6780/squall/" + mSquallSignature.substr(0, 2) + "/" + mSquallSignature.substr(2));
		return std::string("squall://:6780/squall#" + signature->str());
	} else {
		return "";
	}
}

std::optional<Squall::Signature> AssetsHandler::refreshSquallRepository(std::filesystem::path pathToAssets) {
	SquallAssetsGenerator assetsGenerator{Squall::Repository(mSquallRepositoryPath), std::move(pathToAssets)};

	auto rootSignatureResult = assetsGenerator.generateFromAssets("cyphesis-" + ruleset_name);
	if (rootSignatureResult) {
		mState.withState([rootSignatureResult](auto state) {
			state->mSquallSignature = *rootSignatureResult;
		});
	}
	return rootSignatureResult;
}


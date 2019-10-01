/*
 Copyright (C) 2001,2002  Martin Pollard (Xmp), Lakin Weckerd (nikal)

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

//This should be included only to the file, where main function is defined.
#include "MainWrapper.h"

#include "Application.h"
#include "framework/Tokeniser.h"

#ifdef _WIN32
#include "platform/platform_windows.h"

#include <iostream>
#include <fstream>
#include <ostream>
#else
#endif

extern "C"
int main(int argc, char **argv)
{
	int exitStatus(0);
	bool exit_program = false;
	std::string prefix;
	std::string homeDir;
	Ember::Application::ConfigMap configMap;
#ifndef __WIN32__
	if (argc > 1)
	{
		(argv)++;
		argc--;
		while (argc > 0)
		{
			std::string arg = std::string(argv[0]);
			argv++;
			argc--;
			if (arg == "-v" || arg == "--version")
			{
				std::cout << "Ember version: " << VERSION << std::endl;
				exit_program = true;
			}
			else if (arg == "-h" || arg == "--help")
			{
				std::cout << "-h, --help    - display this message" << std::endl;
				std::cout << "-v, --version - display version info" << std::endl;
				std::cout << "--home <path> - sets the home directory to something different than the default (XDG Base Directory Specification on *NIX systems, $APPDATA\\Ember on win32 systems)" << std::endl;
				std::cout << "-p <path>, --prefix <path> - sets the prefix to something else than the one set at compilation (only valid on *NIX systems)" << std::endl;
				std::cout << "--config <section>:<key> <value> - allows you to override config file settings. See the ember.conf file for examples. (~/.config/ember/ember.conf on *NIX systems)" << std::endl;
				exit_program = true;
				break;
			}
			else if (arg == "-p" || arg == "--prefix")
			{
				if (!argc)
				{
					std::cout << "You didn't supply a prefix.";
					exit_program = true;
				}
				else
				{
					prefix = std::string(argv[0]);
					argv++;
					argc--;
				}

			}
			else if (arg == "--home")
			{
				if (!argc)
				{
					std::cout << "You didn't supply a home directory.";
					exit_program = true;
				}
				else
				{
					homeDir = std::string(argv[0]);
					argv++;
					argc--;
				}

			}
			else if (arg == "--config")
			{
				if (argc < 2)
				{
					std::cout << "You didn't supply any config arguments.";
				}
				else
				{
					std::string fullkey(argv[0]);
					std::string value(argv[1]);
					Ember::Tokeniser tokeniser(fullkey, ":");
					if (!tokeniser.remainingTokens().empty())
					{
						std::string category(tokeniser.nextToken());
						if (!tokeniser.remainingTokens().empty())
						{
							std::string key(tokeniser.nextToken());
							configMap[category][key] = value;
						}
					}
				}
			}
		}
	}

#if !defined(__WIN32__) && !defined(__APPLE__)
	if (exit_program)
	{
		return 0;
	}
#endif

#ifdef ENABLE_BINRELOC
	if (prefix == "")
	{
		BrInitError error;

		if (br_init (&error) == 0 && error != BR_INIT_ERROR_DISABLED)
		{
			printf ("Warning: BinReloc failed to initialize (error code %d)\n", error);
			printf ("Will fallback to hardcoded default path.\n");
		}

		char* br_prefixdir = br_find_prefix(PREFIX);
		const std::string prefixDir(br_prefixdir);
		free(br_prefixdir);
		prefix = prefixDir;
	}

#endif

#endif

	try
	{
	//put the application object in its own scope so it gets destroyed before we signal all clear
		{
			if (prefix.empty()) {
				std::cout << "Starting Ember version " << VERSION << "." << std::endl;
			} else {
				std::cout << "Starting Ember version " << VERSION << " with prefix '" << prefix << "'." << std::endl;
			}

			// Create application object
			Ember::Application app(prefix, homeDir, configMap);

			app.registerComponents();

			// Initialize all Ember services needed for this application
			app.prepareComponents();
			app.initializeServices();

			app.start();
		}
	} catch (const std::exception& ex)
	{
		std::cerr << "Unexpected error, aborting.\n\r\t" << ex.what() << std::endl;
		exitStatus = 1;
	}
    std::cout << "Ember shut down successfully." << std::endl;

	return exitStatus;
}

add_library(cyphesis-common
        const.cpp
        newid.cpp
        id.cpp
        log.cpp
        debug.cpp
        type_utils.cpp
        utils.cpp
        operations.cpp
        custom.cpp
        client_socket.cpp
        globals.cpp
        Database.cpp
        system.cpp
        system_net.cpp
        system_uid.cpp
        system_prefix.cpp
        serialno.cpp
        Property.cpp
        Router.cpp
        AtlasFileLoader.cpp
        Monitors.cpp
        Variable.cpp
        AtlasStreamClient.cpp
        ClientTask.cpp
        Link.cpp
        Shaker.cpp
        RuleTraversalTask.cpp
        FileSystemObserver.cpp
        AssetsManager.cpp
        RepeatedTask.cpp
        MainLoop.cpp
        net/CommHttpClient.cpp
        net/HttpHandling.cpp
        FormattedXMLWriter.cpp
        net/SquallHandler.cpp
        AtlasFactories.cpp
        PropertyUtil.cpp
)

target_link_libraries(cyphesis-common PUBLIC
        wfmath
        Atlas
        AtlasCodecs
        AtlasNet
        AtlasMessage
        AtlasObjects
        AtlasFilters
        varconf
        external-dirmonitor
        external-remotery
        external-sqlite3pp
        libsigcpp::sigc++
        libgcrypt::libgcrypt
        external-libxdg-basedir
        spdlog::spdlog
        external-saf
        external-binreloc
        external-bytesize
        Microsoft.GSL::GSL
)

target_compile_features(cyphesis-common PUBLIC cxx_std_20)

target_compile_definitions(cyphesis-common PUBLIC
        -DBINDIR="${CMAKE_INSTALL_FULL_BINDIR}"
        -DDATADIR="${CMAKE_INSTALL_FULL_DATADIR}"
        -DSYSCONFDIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}"
        -DLOCALSTATEDIR="${CMAKE_INSTALL_FULL_LOCALSTATEDIR}"
        -DPACKAGE_BUGREPORT="${PACKAGE_BUGREPORT}")


set(db_files
        Storage.cpp
        DatabaseSQLite.cpp)

if (CYPHESIS_USE_POSTGRES)
    list(APPEND db_files
            DatabasePostgres.cpp
            CommPSQLSocket.cpp)

endif (CYPHESIS_USE_POSTGRES)

add_library(cyphesis-db ${db_files})

if (CYPHESIS_USE_POSTGRES)
    target_link_libraries(cyphesis-db PUBLIC PostgreSQL::PostgreSQL)
endif (CYPHESIS_USE_POSTGRES)

target_link_libraries(cyphesis-db PUBLIC
        SQLite::SQLite3
        external-remotery
        cyphesis-common
)






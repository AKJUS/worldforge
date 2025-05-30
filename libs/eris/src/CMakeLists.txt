set(SOURCE_FILES
        Eris/Account.cpp
        Eris/Avatar.cpp
        Eris/BaseConnection.cpp
        Eris/Calendar.cpp
        Eris/Connection.cpp
        Eris/Entity.cpp
        Eris/EntityRef.cpp
        Eris/EventService.cpp
        Eris/Factory.cpp
        Eris/Lobby.cpp
        Eris/Log.cpp
        Eris/MetaQuery.cpp
        Eris/Metaserver.cpp
        Eris/Person.cpp
        Eris/Redispatch.cpp
        Eris/Response.cpp
        Eris/Room.cpp
        Eris/ServerInfo.cpp
        Eris/StreamSocket.cpp
        Eris/Task.cpp
        Eris/TransferInfo.cpp
        Eris/TypeBoundRedispatch.cpp
        Eris/TypeInfo.cpp
        Eris/TypeService.cpp
        Eris/View.cpp
        Eris/ActiveMarker.cpp
        Eris/World.cpp
)

set(HEADER_FILES
        Eris/Account.h
        Eris/Avatar.h
        Eris/BaseConnection.h
        Eris/Calendar.h
        Eris/Connection.h
        Eris/Entity.h
        Eris/EntityRef.h
        Eris/EventService.h
        Eris/Exceptions.h
        Eris/Factory.h
        Eris/iround.h
        Eris/Lobby.h
        Eris/Log.h
        Eris/LogStream.h
        Eris/MetaQuery.h
        Eris/Metaserver.h
        Eris/Person.h
        Eris/Redispatch.h
        Eris/Response.h
        Eris/Room.h
        Eris/Router.h
        Eris/ServerInfo.h
        Eris/SpawnPoint.h
        Eris/StreamSocket.h
        Eris/StreamSocket_impl.h
        Eris/Task.h
        Eris/TransferInfo.h
        Eris/TypeBoundRedispatch.h
        Eris/TypeInfo.h
        Eris/Types.h
        Eris/TypeService.h
        Eris/View.h
        Eris/WaitFreeQueue.h
        Eris/ActiveMarker.h
        Eris/Usage.h
        Eris/World.h
)

wf_add_library(${LIBNAME} SOURCE_FILES HEADER_FILES)

target_link_libraries(${LIBNAME} PUBLIC
        AtlasObjects
        AtlasNet
        libsigcpp::sigc++
        Boost::headers
        spdlog::spdlog
        wfmath)

if (UNIX)
    target_link_libraries(${LIBNAME} PUBLIC
            Threads::Threads)
endif ()
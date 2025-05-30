if (NOT WF_USE_WIDGET_PLUGINS)
    set(STATIC_WIDGETS
            ServerWidget.cpp
            Quit.cpp
            InspectWidget.cpp
            Help.cpp
            IngameChatWidget.cpp
            ContainerWidget.cpp
            EntityCreatorWidget.cpp
            AssetsSyncWidget.cpp)
endif ()


add_library(ember-widgets STATIC

        ActionBarInput.cpp
        ActionBarIcon.cpp
        ActionBarIconSlot.cpp
        ActionBarIconDragDropTarget.cpp
        ActionBarIconManager.cpp
        AssetsManager.cpp
        ColouredListItem.cpp
        Compass.cpp
        ConsoleAdapter.cpp
        EntityCEGUITexture.cpp
        EntityCreatorMovement.cpp
        EntityCreatorMovementBridge.cpp
        EntityEditor.cpp
        EntityIcon.cpp
        EntityIconDragDropPreview.cpp
        EntityIconDragDropTarget.cpp
        EntityIconManager.cpp
        EntityIconSlot.cpp
        IconBar.cpp
        IconBase.cpp
        ListHolder.cpp
        LoadingBar.cpp
        EntityCreatorTypeHelper.cpp
        MeshInfoProvider.cpp
        ModelRenderer.cpp
        MovableObjectRenderer.cpp
        OgreEntityRenderer.cpp
        QuaternionAdapter.cpp
        QuickHelp.cpp
        QuickHelpCursor.cpp
        HelpMessage.cpp
        WorldLoadingScreen.cpp
        Vector3Adapter.cpp
        Widget.cpp
        WidgetDefinitions.cpp
        AtlasHelper.cpp
        ModelEditHelper.cpp
        EntityTextureManipulator.cpp
        LabelAction.cpp
        icons/Icon.cpp
        icons/IconImageStore.cpp
        icons/IconManager.cpp
        icons/IconRenderer.cpp
        icons/IconStore.cpp
        adapters/atlas/AdapterFactory.cpp
        adapters/atlas/AreaAdapter.cpp
        adapters/atlas/CustomAdapter.cpp
        adapters/atlas/ListAdapter.cpp
        adapters/atlas/MapAdapter.cpp
        adapters/atlas/NumberAdapter.cpp
        adapters/atlas/NumberRangeAdapter.cpp
        adapters/atlas/OrientationAdapter.cpp
        adapters/atlas/PolygonAdapter.cpp
        adapters/atlas/Position2DAdapter.cpp
        adapters/atlas/PositionAdapter.cpp
        adapters/atlas/SizeAdapter.cpp
        adapters/atlas/StaticAdapter.cpp
        adapters/atlas/TerrainModAdapter.cpp
        adapters/eris/RuleTreeAdapter.cpp
        adapters/ogre/ResourceListAdapter.cpp
        adapters/ValueTypeHelper.cpp
        adapters/StringListAdapter.cpp
        TexturePair.cpp
        EntityTooltip.cpp
        EntityCreatorCreationInstance.cpp
        HitDisplayer.cpp
        TextNodeRenderer.cpp
        adapters/ModelDefinitionsAdapter.cpp
        adapters/atlas/ScaleAdapter.cpp
        adapters/atlas/EntityRefAdapter.cpp
        ContainerView.cpp
        ${STATIC_WIDGETS}
)

target_precompile_headers(ember-widgets
        REUSE_FROM
        ember-ogre)

target_link_libraries(ember-widgets PUBLIC
        ember-framework
        ember-ogre
        ember-cegui)

#wf_add_plugin(ServerWidget
#        ServerWidget.cpp
#        )
#wf_add_plugin(Quit
#        Quit.cpp
#        )
#wf_add_plugin(InspectWidget
#        InspectWidget.cpp
#        )
#
#wf_add_plugin(Help
#        Help.cpp
#        )
#
#wf_add_plugin(ContainerWidget
#        ContainerWidget.cpp
#        )
#
#wf_add_plugin(IngameChatWidget
#        IngameChatWidget.cpp
#        )

file(GLOB LAYOUT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.layout")
install(FILES ${LAYOUT_FILES} DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/ember/gui/cegui/datafiles/layouts/)

file(GLOB ATLAS_ADAPTER_LAYOUT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/adapters/atlas/*.layout")
install(FILES ${ATLAS_ADAPTER_LAYOUT_FILES} DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/ember/gui/cegui/datafiles/layouts/adapters/atlas/)

file(GLOB REPRESENTATION_LAYOUT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/representations/*.layout")
install(FILES ${REPRESENTATION_LAYOUT_FILES} DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/ember/gui/cegui/datafiles/layouts/representations)

file(GLOB LUA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.lua")
install(FILES ${LUA_FILES} DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/ember/scripting/lua/)


add_library(ember-BindingsAtlasAdapters STATIC
        adapters/atlas/bindings/lua/BindingsAtlasAdapters.cpp
)

target_link_libraries(ember-BindingsAtlasAdapters PUBLIC
        ember-framework
        ember-external-sol2
        ember-cegui
        AtlasObjects
        varconf
)
target_precompile_headers(ember-BindingsAtlasAdapters
        REUSE_FROM
        ember-BindingsLua
)
if (MSVC)
    #Need to allow for a lot of symbols when compiling the bindings on MSVC.
    target_compile_options(ember-BindingsAtlasAdapters PRIVATE "/bigobj")
endif ()

add_library(ember-BindingsRepresentations STATIC
        representations/LayoutHelper.cpp
        representations/bindings/lua/BindingsRepresentations.cpp
)

target_link_libraries(ember-BindingsRepresentations PUBLIC
        ember-framework
        ember-external-sol2
        ember-cegui
        ember-widgets
        AtlasObjects
        varconf
)
target_precompile_headers(ember-BindingsRepresentations
        REUSE_FROM
        ember-BindingsLua
)
if (MSVC)
    #Need to allow for a lot of symbols when compiling the bindings on MSVC.
    target_compile_options(ember-BindingsRepresentations PRIVATE "/bigobj")
endif ()

target_compile_definitions(ember-widgets PUBLIC PLUGIN_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")

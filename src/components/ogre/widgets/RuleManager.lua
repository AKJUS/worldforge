RuleManager = {}

function RuleManager:buildWidget()

	self.widget = guiManager:createWidget()

	local setup = function()
	
		self.ruleTree = tolua.cast(self.widget:getWindow("RuleList"), "CEGUI::Tree")
		self.ruleTree:subscribeEvent("SelectionChanged", self.RuleList_SelectionChanged, self)
	
		self.codecTypeCombobox = CEGUI.toCombobox(self.widget:getWindow("CodecType"))
		
		local item = Ember.OgreView.Gui.ColouredListItem:new("XML", 0)
		self.codecTypeCombobox:addItem(item)
		item = Ember.OgreView.Gui.ColouredListItem:new("Bach", 1)
		self.codecTypeCombobox:addItem(item)
		item = Ember.OgreView.Gui.ColouredListItem:new("Packed", 2)
		self.codecTypeCombobox:addItem(item)
		self.codecTypeCombobox:setItemSelectState(0, true)
		self.codecTypeCombobox:setSingleClickEnabled(true)
		self.codecTypeCombobox:subscribeEvent("ListSelectionChanged", self.CodecType_ListSelectionChanged, self)
	
		
		self.ruleInfoText = CEGUI.toMultiLineEditbox(self.widget:getWindow("RuleInfoText"))
	
		self.ruleAdapter = Ember.OgreView.Gui.Adapters.Eris.RuleTreeAdapter:new_local(self.avatar:getConnection(), self.avatar:getId(), self.ruleTree)
		local loadingOverlay = self.widget:getWindow("LoadingOverlay")
		local refreshButton = self.widget:getWindow("Refresh")

		local refresh = function()
			refreshButton:setEnabled(false)
			self.ruleAdapter:refresh("root")
			
			loadingOverlay:setVisible(true)
			loadingOverlay:setText("Getting rules from server.")
			
			connect(self.connectors, self.ruleAdapter.EventNewRuleReceived, function(numberOfRules)
				loadingOverlay:setText("Getting rules from server.\n" .. numberOfRules .. " rules received.")
				end
			)
			connect(self.connectors, self.ruleAdapter.EventAllRulesReceived, function()
					loadingOverlay:setVisible(false)
					refreshButton:setEnabled(true)
				end
			)
		end
		
		refresh()

		self.widget:getWindow("Refresh"):subscribeEvent("Clicked", refresh)
		
		
		self.widget:getWindow("SendToServerButton"):subscribeEvent("Clicked", self.SendToServerButton_Clicked, self)


		local ruleUpdateOverlay = self.widget:getWindow("RuleUpdateOverlay")
		ruleUpdateOverlay:subscribeEvent("MouseEntersSurface", function()
			ruleUpdateOverlay:fireEvent("StartHideTransition", CEGUI.WindowEventArgs:new_local(ruleUpdateOverlay))
			return true
		end)
					
		self.editor = Ember.OgreView.Authoring.RuleEditor:new_local(self.avatar)
		
		connect(self.connectors, self.editor.EventRuleCreated, function(refno)
			ruleUpdateOverlay:setVisible(true)
			ruleUpdateOverlay:setText("New rule created on server.")
			end
		)
		connect(self.connectors, self.editor.EventRuleUpdated, function(refno)
			ruleUpdateOverlay:setVisible(true)
			ruleUpdateOverlay:setText("Existing rule updated on server.")
			end
		)
		connect(self.connectors, self.editor.EventRuleEditError, function(refno)
			ruleUpdateOverlay:setVisible(true)
			ruleUpdateOverlay:setText("Error when updating or creating rule.")
			end
		)
					
					
		self.widget:enableCloseButton()
	end

	connect(self.connectors, self.widget.EventFirstTimeShown, setup)
	self.widget:loadMainSheet("RuleManager.layout", "RuleManager")
	self.widget:registerConsoleVisibilityToggleCommand("ruleManager")

end

function RuleManager:CodecType_ListSelectionChanged()
	local item = self.codecTypeCombobox:getSelectedItem()
	if item ~= nil then
		local selectId = item:getID()
		if selectId == 0 then
			self.codecClass = Atlas.Codecs.XML
		elseif selectId == 1 then
			self.codecClass = Atlas.Codecs.Bach
		else
			self.codecClass = Atlas.Codecs.Packed
		end
		self:printRule()
	end
end

function RuleManager:sendRuleToServer()

	local ruleUpdateOverlay = self.widget:getWindow("RuleUpdateOverlay")

	local outstream = std.stringstream:new_local(self.ruleInfoText:getText())
	local decoder = Ember.AtlasObjectDecoder:new_local(self.avatar:getConnection():getFactories())

	local istream = tolua.cast(outstream, "std::istream")
	local ostream = tolua.cast(std.stringstream:new_local(), "std::ostream")
	local bridge = tolua.cast(decoder, "Atlas::Bridge")

	local codec = self.codecClass:new_local(istream, ostream, bridge)
	codec:poll()
	
	local parsedObject = decoder:getLastObject()
	
	if parsedObject:isValid() then
		ruleUpdateOverlay:setVisible(true)
		ruleUpdateOverlay:setText("Sending rule to server.")
	
		self.editor:updateOrCreateRule(parsedObject)
	end
end

function RuleManager:SendToServerButton_Clicked(args)

	self:sendRuleToServer()
	return true
end

function RuleManager:printRule()
	local rawRuleData = self.ruleAdapter:getSelectedRule()
	
	if rawRuleData:isValid() then
	
		local outstream = std.stringstream:new_local()
		local decoder = Atlas.Message.QueuedDecoder:new_local()
    	local ostream = tolua.cast(outstream, "std::ostream")
    	local istream = tolua.cast(Atlas.Message.QueuedDecoder:new_local(), "std::istream")

		local codec = self.codecClass:new_local(istream, ostream, tolua.cast(decoder, "Atlas::Bridge"))
		local formatter = Atlas.Formatter:new_local(ostream, tolua.cast(codec, "Atlas::Bridge"))
		local encoder = Atlas.Message.Encoder:new_local(tolua.cast(formatter, "Atlas::Bridge"))
		local message = Atlas.Message.MapType:new_local()
		rawRuleData:get():addToMessage(message)
		formatter:streamBegin();
		encoder:streamMessageElement(message);
	
		formatter:streamEnd();
	
		self.ruleInfoText:setText(outstream:str())
	end
end

function RuleManager:RuleList_SelectionChanged(args)

	self:printRule()
	return true
end

function RuleManager:shutdown()
	disconnectAll(self.connectors)
	guiManager:destroyWidget(self.widget)
end

RuleManager.gotAvatarConnector = createConnector(emberServices:getServerService().GotAvatar):connect(function(avatar)
		ruleManager = {connectors={}, codecClass=Atlas.Codecs.XML, avatar=avatar}
		setmetatable(ruleManager, {__index = RuleManager})
		
		ruleManager:buildWidget()
		connect(ruleManager.connectors, emberServices:getServerService().DestroyedAvatar, function()
				ruleManager:shutdown()
				ruleManager = nil
			end
		)
	end
)



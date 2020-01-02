-----------------------------------------


-----------------------------------------
Inventory = {}

function Inventory:AddedEntityToInventory(entity)
	local entityIconWrapper = self:createIcon(entity)
	if entityIconWrapper ~= nil then
		local slotWrapper = self:getFreeSlot()
		local slot = slotWrapper.slot
		slot:addEntityIcon(entityIconWrapper.entityIcon)
		local entityIconBucket = {}
		if self.icons[entity:getId()] == nil then
			self.icons[entity:getId()] = entityIconBucket
		else
			entityIconBucket = self.icons[entity:getId()]
		end
		table.insert(entityIconBucket, entityIconWrapper)
		for k,v in pairs(self.newEntityListeners) do
			v(entity)
		end
	end
end

function Inventory:RemovedEntityFromInventory(entity)
	local entityIconBucket = self.icons[entity:getId()]
	if entityIconBucket ~= nil then
		for k,v in pairs(entityIconBucket) do 
			local entityIconWrapper = v
			entityIconWrapper.entityIcon:setSlot(nil)
      --Reset the entityIcon to let the wrapper know that the icon is being deleted. This fixes an issue where the callback for MouseLeaving would try to access the icon while it was being destroyed.
      local entityIcon = entityIconWrapper.entityIcon
			entityIconWrapper.entityIcon = nil
			--guiManager:getIconManager():destroyIcon(entityIconWrapper.entityIcon:getIcon())
			self.entityIconManager:destroyIcon(entityIcon)
		end
	end
	self.icons[entity:getId()] = nil
end
	
function Inventory:getFreeSlot()
	--see if there's any free slots
	for k,v in pairs(self.slots) do 
		if v.slot:getEntityIcon() == nil then
			return v
		end
	end
	--if we couldn't find a free one, add one
	return self:addSlot()
end

function Inventory:addSlot()
	local yPosition = math.floor(self.slotcounter / self.columns)
	local xPosition = self.slotcounter - math.floor(self.slotcounter/self.columns)*self.columns  --lua 5.0 can't do modulus, in 5.1 we would have done: self.slotcounter % self.columns
	
	
	
	self.slotcounter = self.slotcounter + 1
	
	local slot = self.entityIconManager:createSlot(self.iconsize)
	slot:getWindow():setPosition(CEGUI.UVector2(CEGUI.UDim(0, self.iconsize * xPosition), CEGUI.UDim(0, self.iconsize * yPosition)))
	self.iconContainer:addChild(slot:getWindow())
	local slotWrapper = {slot = slot}
	table.insert(self.slots, slotWrapper)
	slotWrapper.entityIconDropped = function(entityIcon)
		local oldSlot = entityIcon:getSlot()
		slotWrapper.slot:addEntityIcon(entityIcon)
		if oldSlot ~= nil then
			oldSlot:notifyIconDraggedOff(entityIcon)
		end
	end
	
	slotWrapper.entityIconDropped_connector = createConnector(slot.EventIconDropped):connect(slotWrapper.entityIconDropped)
	
	return slotWrapper
end

function Inventory:showMenu(args, entityIconWrapper)

    local entity = entityIconWrapper.entity
	guiManager:EmitEntityAction("pick", entity)

end

-- function Inventory:input_MouseButtonReleased()
-- 	if self.menu.menuShown then
-- 		self.menu.container:setVisible(false)
-- 		self.menu.menuShown = false
-- 	end
-- end

function Inventory:createIcon(entity)

	local icon = guiManager:getIconManager():getIcon(self.iconsize, entity)
	
	if icon ~= nil then
		local name = entity:getType():getName() .. " (" .. entity:getId() .. " : " .. entity:getName() .. ")"
		local entityIconWrapper = {}
		entityIconWrapper.entityIcon = self.entityIconManager:createIcon(icon, entity, self.iconsize)
		entityIconWrapper.entityIcon:getImage():setTooltip(guiManager:getEntityTooltip():getTooltipWindow())
		entityIconWrapper.entityIcon:getImage():setTooltipText(entity:getId())
		entityIconWrapper.entity = entity
		entityIconWrapper.mouseEnters = function(args)
		  if entityIconWrapper.entityIcon then
		    entityIconWrapper.entityIcon:getImage():setProperty("FrameEnabled", "true")
			end
			return true
		end
		entityIconWrapper.mouseLeaves = function(args)
      if entityIconWrapper.entityIcon then
  			entityIconWrapper.entityIcon:getImage():setProperty("FrameEnabled", "false")
  		end
			return true
		end
		entityIconWrapper.mouseClick = function(args)
			self:showMenu(args, entityIconWrapper)
			return true
		end
		entityIconWrapper.entityIcon:getDragContainer():subscribeEvent("MouseClick", entityIconWrapper.mouseClick)
		entityIconWrapper.entityIcon:getDragContainer():subscribeEvent("MouseEntersSurface", entityIconWrapper.mouseEnters)
		entityIconWrapper.entityIcon:getDragContainer():subscribeEvent("MouseLeavesSurface", entityIconWrapper.mouseLeaves)
		return entityIconWrapper
	else 
		return nil
	end
end

function Inventory:buildWidget(avatarEntity)
	
	self.widget = guiManager:createWidget()
	
	--inventory has transitions, if we don't set this to false, every time it gets activated a 
	--visible "alpha pop" would happen and then the transition would continue
	self.widget:setIsActiveWindowOpaque(false)
	
	self.widget:loadMainSheet("Inventory.layout", "Inventory")
	
	self.entityIconManager = guiManager:getEntityIconManager()
	
	self.iconContainer = self.widget:getWindow("IconContainer");
	
	self.widget:enableCloseButton()
	
	self.menu.container = guiManager:createWindow("DefaultWindow")
	self.menu.container:setSize(CEGUI.USize(CEGUI.UDim(0, 50), CEGUI.UDim(0, 200)))
	self.menu.container:setClippedByParent(false)
	self.menu.container:setAlwaysOnTop(true)
	
	self.menu.innercontainer = guiManager:createWindow("DefaultWindow")
	self.menu.innercontainer:setSize(CEGUI.USize(CEGUI.UDim(0, 50), CEGUI.UDim(0, 200)))
	self.menu.innercontainer:setClippedByParent(false)
	self.menu.stackableContainer = Ember.OgreView.Gui.StackableContainer:new_local(self.menu.innercontainer)
	self.menu.stackableContainer:setInnerContainerWindow(self.menu.innercontainer)
	self.menu.container:addChild(self.menu.innercontainer)
	self.menu.innercontainer:setPosition(CEGUI.UVector2(CEGUI.UDim(0, 10), CEGUI.UDim(1, -self.iconsize)))
	
	self.menu.hide = function()
		guiManager:getMainSheet():removeChild(self.menu.container)
		self.menu.menuShown = false
	end
	
	self.menu.mouseLeaves = function(args)
		if self.menu.menuShown then
			self.menu.hide()
		end
		return true
	end
	
	self.menu.container:subscribeEvent("MouseLeavesArea", self.menu.mouseLeaves)
	
	
	--add default buttons

	self.menu.dropButton = guiManager:createWindow("EmberLook/Button")
	self.menu.dropButton:setSize(CEGUI.USize(CEGUI.UDim(1, 0), CEGUI.UDim(0, 25)))
	self.menu.dropButton:setText("drop")
	self.menu.dropButton_MouseClick = function(args)
		if self.menu.activeEntityWrapper ~= nil then
			if self.menu.activeEntityWrapper.entity ~= nil then
				emberOgre:getWorld():getAvatar():getErisAvatar():drop(self.menu.activeEntityWrapper.entity)
			end
		end
		self.menu.hide()
		return true
	end
	self.menu.dropButton:subscribeEvent("Clicked", self.menu.dropButton_MouseClick)
	self.menu.innercontainer:addChild(self.menu.dropButton)

    for i = 0, 10 do
        local button = guiManager:createWindow("EmberLook/Button")
        button:setSize(CEGUI.USize(CEGUI.UDim(1, 0), CEGUI.UDim(0, 25)))
        local buttonWrapper = {}
        buttonWrapper.button = button
        buttonWrapper.clicked = function()
            buttonWrapper.clickedHandler()
        end
        buttonWrapper.button:subscribeEvent("MouseButtonUp", buttonWrapper.clicked)
        self.useButtons[table.getn(self.useButtons) + 1] = buttonWrapper

        self.menu.innercontainer:addChild(button)
    end
	

	self.helper = Ember.OgreView.Gui.EntityIconDragDropPreview:new(emberOgre:getWorld())
	--User has dragged an entityIcon from the inventory to the world
	self.DragDrop = Ember.OgreView.Gui.EntityIconDragDropTarget:new(root)
	local dragDrop_DraggedOver = function(entityIcon)
		if entityIcon ~= nil then
			if entityIcon:getImage() ~= nil then
				--alpha is already low when dragging, so 0.7 of an already reduced alpha
				entityIcon:getImage():setAlpha(0.7)
				self.helper:createPreview(entityIcon)
			end
		end
	end
	connect(self.connectors, self.DragDrop.EventIconEntered, dragDrop_DraggedOver)
	
	--User has dragged an entityIcon over the world, and onto another window
	local dragDrop_DragLeaves = function(entityIcon)
		if entityIcon ~= nil then
			if entityIcon:getEntity() ~= nil then
				entityIcon:getImage():setAlpha(1.0)
				self.helper:cleanupCreation()
			end
		end
	end
	connect(self.connectors, self.DragDrop.EventIconLeaves, dragDrop_DragLeaves)
	
	--Responds when preview model has been released on the world
	local dragDrop_Finalize = function(emberEntity)
		if emberEntity ~= nil then
			local offset = self.helper:getDropOffset()
			local orientation = self.helper:getDropOrientation()
			emberOgre:getWorld():getAvatar():getErisAvatar():drop(emberEntity, offset, orientation)
		end
	end
	connect(self.connectors, self.helper.EventEntityFinalized, dragDrop_Finalize)
	
	--The icon container will create a child window with the suffix "__auto_container__" which will catch all events. We need to attach to that.
	local iconContainer_inner = self.widget:getWindow("IconContainer"):getChild("__auto_container__")
	
	self.IconContainerDragDrop = Ember.OgreView.Gui.EntityIconDragDropTarget:new(iconContainer_inner)
	connect(self.connectors, self.IconContainerDragDrop.EventIconDropped, function(entityIcon)
		if entityIcon ~= nil then
			if entityIcon:getEntity() ~= nil then
				local slotWrapper = self:getFreeSlot()
				slotWrapper.entityIconDropped(entityIcon)
			end
		end
	end)
	
	
	
	self.menu.container:setVisible(true)
	
	
	connect(self.connectors, emberOgre:getWorld():getAvatar().EventAddedEntityToInventory, self.AddedEntityToInventory, self)
	connect(self.connectors, emberOgre:getWorld():getAvatar().EventRemovedEntityFromInventory, self.RemovedEntityFromInventory, self)
	
	self.widget:registerConsoleVisibilityToggleCommand("inventory")
	self.avatarEntity = avatarEntity
	self:setupDoll(avatarEntity)
--[[
	connect(self.connectors, avatarEntity.Changed, function(self, keys)
			self:updateDoll()
		end
	, self)
	]]--
	self.widget:show()
-- 	connect(self.connectors, Ember.Input:getSingleton().EventMouseButtonReleased, self.input_MouseButtonReleased, self)
--	guiManager:getMainSheet():addChild(self.menu.container)

	
end

function Inventory:createAttachmentSlot(avatarEntity, dollSlot, attachment)
-- 	self.doll.torso = self:createDollSlot("body", self.doll.image:getChild("Torso"), "Drop an entity here to attach it to the torso.")
	dollSlot.droppedHandler = function(entityIcon)
		if dollSlot.isValidDrop(entityIcon) then
			emberOgre:getWorld():getAvatar():getErisAvatar():wield(entityIcon:getEntity(), attachment)
			local icon = dollSlot.slot:getEntityIcon()
			if icon ~= nil then
				local slotWrapper = self:getFreeSlot()
				local slot = slotWrapper.slot
				slot:addEntityIcon(icon)
			end
			dollSlot.slot:addEntityIcon(entityIcon)
		end
	end
	dollSlot.entityIconDropped_connector = createConnector(dollSlot.slot.EventIconDropped):connect(dollSlot.droppedHandler)
	dollSlot.observer = Ember.AttributeObserver:new_local(avatarEntity, dollSlot.attributePath, ".")
	dollSlot.attributeChanged = function(element)
		local result, entityId = Eris.Entity:extractEntityId(element, entityId)
		if result then
			local slotUpdateFunc = function()
				local entityBucket = self.icons[entityId]

				if entityBucket ~= nil then
					local icon = entityBucket[1].entityIcon
					if icon ~= nil then
						--check that we've not already have added the icon to this slot
						if dollSlot.slot:getEntityIcon() ~= icon then
							local oldIcon = dollSlot.slot:removeEntityIcon()
							dollSlot.slot:addEntityIcon(icon)
							if oldIcon ~= nil then
								local slotWrapper = self:getFreeSlot()
								local slot = slotWrapper.slot
								slot:addEntityIcon(oldIcon)
							end
						end
					end
				end
			end

			--Either we have created an icon for the entity yet, or we have to wait a little until it's available			
			local entityBucket = self.icons[entityId]
			if entityBucket ~= nil then
				slotUpdateFunc()
			else
				local delayedUpdater = function(newEntity)
					if newEntity:getId() == entityId then
						slotUpdateFunc()
						self.newEntityListeners[dollSlot.attributePath] = nil
					end
				end
				self.newEntityListeners[dollSlot.attributePath] = delayedUpdater
			end
		end
	end
	dollSlot.attributeChanged_connector = createConnector(dollSlot.observer.EventChanged):connect(dollSlot.attributeChanged)
	
	dollSlot.iconDraggedOff = function(entityIcon)
		--do unwield stuff
		emberOgre:getWorld():getAvatar():getErisAvatar():wield(nil, attachment)
	end
	dollSlot.iconDraggedOff_connector = createConnector(dollSlot.slot.EventIconDraggedOff):connect(dollSlot.iconDraggedOff)
	
	dollSlot.observer:forceEvaluation()
	
--	dollSlot.newEntityCreated = function(newEntity)
--		if avatarEntity:hasProperty("outfit") then
--			dollSlot.attributeChanged(avatarEntity:valueOfProperty("outfit"))
--		end
--	end
	
--	table.insert(self.newEntityListeners, dollSlot.newEntityCreated)
	
-- 	dollSlot.attributeChanged(avatarEntity:valueOfProperty("outfit"))
end

function Inventory:setupDoll(avatarEntity)
	self.doll = {}
	self.doll.image = self.widget:getWindow("DollImage")
	self.doll.renderer = Ember.OgreView.Gui.ModelRenderer:new(self.doll.image, "doll")
	self.doll.renderer:setActive(false)

	self.doll.handPrimary = self:createDollSlot("attached_hand_primary", self.doll.image:getChild("RightHand"), "Drop an entity here to attach it to the primary hand.", "")
	self.doll.handPrimaryAttachmentSlot = self:createAttachmentSlot(avatarEntity, self.doll.handPrimary, "hand_primary")
	
	self.doll.torso = self:createDollSlot("attached_torso", self.doll.image:getChild("Torso"), "Drop an entity here to attach it to the torso.", "torso")
	self.doll.torsoAttachmentSlot = self:createAttachmentSlot(avatarEntity, self.doll.torso, "torso")
	
	self.doll.back = self:createDollSlot("attached_back", self.doll.image:getChild("Back"), "Drop an entity here to attach it to the back.", "back")
	self.doll.backAttachmentSlot = self:createAttachmentSlot(avatarEntity, self.doll.back, "back")

	self.doll.head = self:createDollSlot("attached_head", self.doll.image:getChild("Head"), "Drop an entity here to attach it to the head.", "head")
	self.doll.headAttachmentSlot = self:createAttachmentSlot(avatarEntity, self.doll.head, "head")

	self.doll.legs = self:createDollSlot("attached_legs", self.doll.image:getChild("Legs"), "Drop an entity here to attach it to the legs.", "legs")
	self.doll.legsAttachmentSlot = self:createAttachmentSlot(avatarEntity, self.doll.legs, "legs")
	
	self.doll.feet = self:createDollSlot("attached_feet", self.doll.image:getChild("Feet"), "Drop an entity here to attach it to the feet.", "feet")
	self.doll.feetAttachmentSlot = self:createAttachmentSlot(avatarEntity, self.doll.feet, "feet")
		
	local representationUpdate = function()
		local model = Ember.OgreView.Model.ModelRepresentation:getModelForEntity(avatarEntity)
		if model ~= nil then
			self.doll.renderer:showModel(model:getDefinition())
			self.doll.renderer:setCameraDistance(0.75)
			self.doll.renderer:updateRender()
		end
	end
	
	representationUpdate()
	
	table.insert(self.connectors, createConnector(avatarEntity.EventChangedGraphicalRepresentation):connect(representationUpdate))
	
end

function Inventory:updateDoll()
end


function Inventory:createDollSlot(attributePath, containerWindow, tooltipText, wearRestriction)
	local dollSlot = {}
	dollSlot.slot = self.entityIconManager:createSlot(self.iconsize)
	dollSlot.container = containerWindow
	dollSlot.container:addChild(dollSlot.slot:getWindow())
	dollSlot.slot:getWindow():setInheritsTooltipText(true)
	dollSlot.container:setTooltipText(tooltipText)
	dollSlot.wearRestriction = wearRestriction
	dollSlot.attributePath = attributePath
	dollSlot.allowAny = allowAny
	
	dollSlot.isValidDrop = function(entityIcon)
		if dollSlot.wearRestriction == "" or dollSlot.allowAny then
			return true
		end
		if entityIcon:getEntity():hasProperty("worn") then
			local wornElement = entityIcon:getEntity():valueOfProperty("worn")
			if wornElement:isString() then
				local worn = wornElement:asString()
				if worn == dollSlot.wearRestriction then
					return true
				end
			end
		end
		return false
	end
	
	dollSlot.entityIconDragStart = function(entityIcon)
		if dollSlot.isValidDrop(entityIcon) then
		else
			dollSlot.container:setProperty("FrameEnabled", "false")
		end
	end
	
	dollSlot.entityIconDragStop = function(entityIcon)
		dollSlot.container:setProperty("FrameEnabled", "true")
	end
	
	dollSlot.entityIconDragStart_connector = createConnector(self.entityIconManager.EventIconDragStart):connect(dollSlot.entityIconDragStart)
	dollSlot.entityIconDragStop_connector = createConnector(self.entityIconManager.EventIconDragStop):connect(dollSlot.entityIconDragStop)

	dollSlot.shutdown = function()
		self.entityIconManager:destroySlot(dollSlot.slot)
		dollSlot.entityIconDragStart_connector:disconnect()
		dollSlot.entityIconDragStop_connector:disconnect()
	end

	return dollSlot
end

function Inventory:shutdown()
	disconnectAll(self.connectors)
	deleteSafe(self.helper)
	deleteSafe(self.DragDrop)
	deleteSafe(self.IconContainerDragDrop)
	if self.doll ~= nil then
		if deleteSafe(self.doll.renderer) then
			self.doll.handPrimary.shutdown()
			self.doll.torso.shutdown()
			self.doll.back.shutdown()
			self.doll.head.shutdown()
			self.doll.legs.shutdown()
			self.doll.feet.shutdown()
		end
	end
	for k,v in pairs(self.slots) do
		self.entityIconManager:destroySlot(v.slot)
	end
	for k,v in pairs(self.icons) do
		local iconBucket = v
		for _,bucket in pairs(iconBucket) do
			self.entityIconManager:destroyIcon(bucket.entityIcon)
		end
	end
	
	windowManager:destroyWindow(self.menu.container)
	guiManager:destroyWidget(self.widget)
end

Inventory.createdAvatarEntityConnector = createConnector(emberOgre.EventCreatedAvatarEntity):connect(function(avatarEntity)
		if emberOgre:getWorld():getAvatar():isAdmin() == false then
			inventory = {connectors={},
			    useButtons={},
				iconsize = 32,
				columns = 4,
				iconcounter = 0,
				slotcounter = 0,
				icons = {},
				slots = {},
				menu = {menuShown = false, activeEntityWrapper = nil},
				newEntityListeners = {}}
			setmetatable(inventory, {__index = Inventory})
			inventory:buildWidget(avatarEntity)
			connect(inventory.connectors, avatarEntity.BeingDeleted, function()
					inventory:shutdown()
					inventory = nil
				end
			)
		end
	end
)

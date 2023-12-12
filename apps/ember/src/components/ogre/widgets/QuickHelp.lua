QuickHelp = {}

function QuickHelp.buildWidget()
	quickhelp = {
		helper = nil,
		timeToShowBlurb = 4,
		timeBlurbShown = 0,
		timeBlurbLastUpdate = -1,
		showTransitionStarted = false,
		hideTransitionStarted = false,
		hidden = false,
		updateText_connector = nil,
		updateAlpha_connector = nil
	}
	setmetatable(quickhelp, { __index = QuickHelp })

	quickhelp.helper = Ember.OgreView.Gui.QuickHelpCursor.new()

	quickhelp.updateText_connector = quickhelp.helper.EventUpdateText:connect(quickhelp.updateText, quickhelp)
	quickhelp.toggleVisibility_connector = Ember.OgreView.Gui.QuickHelp.getSingleton().EventToggleWidgetVisibility:connect(quickhelp.toggleVisibility, quickhelp)
	quickhelp:buildCEGUIWidget()
end

function QuickHelp:buildCEGUIWidget()
	self.widget = guiManager:createWidget()
	self.widget:loadMainSheet("QuickHelp.layout", "QuickHelp")

	self.textWindow = self.widget:getWindow("HelpTextBox")
	self.frameWindow = CEGUI.toFrameWindow(self.widget:getMainWindow())

	self.frameWindow:setRollupEnabled(false)

	--self.widget:enableCloseButton()
	self.frameWindow:subscribeEvent("CloseClicked", function()
		self:hideWithTransition()

		return true
	end)

	self.widget:getWindow("Hide"):subscribeEvent("Clicked", function()
		self.hidden = true
		self:hideWithTransition()

		return true
	end)
	self.widget:getWindow("Next"):subscribeEvent("Clicked", function()
		self.helper:nextMessage()
		return true
	end
	)
	self.widget:getWindow("Previous"):subscribeEvent("Clicked", function()
		self.helper:previousMessage()
		return true
	end)
	self:updateButtons()

	self.widget:hide()
end

-- Shows the QuickHelp widget immediately, no transitions
function QuickHelp:show()
	self.frameWindow:fireEvent("InterruptTransitions", CEGUI.WindowEventArgs.new(self.frameWindow))
	self.showTransitionStarted = false
	self.hideTransitionStarted = false

	self.widget:show()
	-- ensure a "valid" alpha is set
	self.frameWindow:setAlpha(1.0)
end

function QuickHelp:showWithTransition()
	-- make sure no other transitions are running
	self.frameWindow:fireEvent("InterruptTransitions", CEGUI.WindowEventArgs.new(self.frameWindow))

	self.showTransitionStarted = true
	self.hideTransitionStarted = false

	self.widget:show()
	self.frameWindow:fireEvent("StartShowTransition", CEGUI.WindowEventArgs.new(self.frameWindow))
end

-- Hides the QuickHelp widget immediately, no transitions
function QuickHelp:hide()
	self.frameWindow:fireEvent("InterruptTransitions", CEGUI.WindowEventArgs.new(self.frameWindow))
	self.showTransitionStarted = false
	self.hideTransitionStarted = false

	self.widget:hide()
	self:disableAlphaConnector()
end

function QuickHelp:hideWithTransition()
	-- make sure no other transitions are running
	self.frameWindow:fireEvent("InterruptTransitions", CEGUI.WindowEventArgs.new(self.frameWindow))

	self.showTransitionStarted = false
	-- make sure we store that we already sent the event, otherwise a never ending event firing starts
	self.hideTransitionStarted = true

	-- by firing this event, we allow looknfeel of the QuickHelp window to act accordingly
	-- (at the time of writing this, it fades the FrameWindow out over 5 seconds)
	self.frameWindow:fireEvent("StartHideTransition", CEGUI.WindowEventArgs.new(self.frameWindow))
end

function QuickHelp:hideWithSlowTransition()
	-- make sure no other transitions are running
	self.frameWindow:fireEvent("InterruptTransitions", CEGUI.WindowEventArgs.new(self.frameWindow))

	self.showTransitionStarted = false
	-- make sure we store that we already sent the event, otherwise a never ending event firing starts
	self.hideTransitionStarted = true

	-- by firing this event, we allow looknfeel of the QuickHelp window to act accordingly
	-- (at the time of writing this, it fades the FrameWindow out over 5 seconds)
	self.frameWindow:fireEvent("StartSlowHideTransition", CEGUI.WindowEventArgs.new(self.frameWindow))
end

function QuickHelp:frameStarted(timeSinceLastUpdate)
	if self.widget:isActive() then
		self:disableAlphaConnector()

		-- ensure the widget is shown at this point no matter what
		if not self.widget:isVisible() then
			self:showWithTransition()
		end

	else
		self.timeBlurbShown = timeSinceLastUpdate + self.timeBlurbShown

		if self.timeBlurbShown > self.timeToShowBlurb then
			if not self.hideTransitionStarted then
				self:hideWithSlowTransition()
			end
		end
	end
end

function QuickHelp:toggleVisibility()
	self.hidden = false
	self:showWithTransition()
end

function QuickHelp:updateText(helpMessage)
	local text = helpMessage.mMessage
	if not self.hidden then
		--Adapt the time the help widget is shown to how many words there are in the text
		local words = 0
		for _ in string.gmatch(text, "[^%s]+") do
			words = words + 1
		end
		--a normal human reads about 250 words per minute, which is ~4 words per second. We'll go for two words per second
		--as the user might not concentrate on the help message at first. Show it no lesser than four seconds
		self.timeToShowBlurb = math.max(words / 2.0, 4)
		self.timeBlurbLastUpdate = -1
		self.timeBlurbShown = 0

		if self.updateAlpha_connector then
			self.updateAlpha_connector:disconnect()
		end
		self.updateAlpha_connector = self.widget.EventFrameStarted:connect(self.frameStarted, self)

		-- even if the widget is already visible at this point, this won't harm things
		self:showWithTransition()
	end

	self.textWindow:setText(text)
	self.widget:getMainWindow():setText("Help - " .. helpMessage.mTitle)
	self:updateButtons()
end

function QuickHelp:disableAlphaConnector()
	self.updateAlpha_connector:disconnect()
	self.updateAlpha_connector = nil
end

function QuickHelp:updateButtons()
	self.widget:getWindow("Next"):setVisible(self.helper:getCursorLocation() < self.helper:getSize())
	self.widget:getWindow("Previous"):setVisible(self.helper:getCursorLocation() > 1)
end

QuickHelp.buildWidget()

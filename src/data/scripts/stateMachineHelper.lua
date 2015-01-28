logMessage("using freeze.lua")

function freezeEverything()
	player:freeze()
	for k, v in pairs(objectManager:getActiveFromPool({MediumBall, SmallBall})) do				
		v:freeze()
	end	
end

function unfreezeEverything()
	player:unfreeze()
	for k, v in pairs(objectManager:getActiveFromPool({MediumBall, SmallBall})) do				
		v:unfreeze()
	end	
end

function disposeEverything()
	logMessage("disposing everything")
	for k, v in pairs(objectManager:getActiveFromPool({MediumBall})) do				
		objectManager:put(MediumBall,v)
	end
	for k, v in pairs(objectManager:getActiveFromPool({SmallBall})) do				
		objectManager:put(SmallBall,v)
	end
	for k, v in pairs(objectManager:getActiveFromPool({Hookshot})) do				
		objectManager:put(Hookshot,v)
	end
end

-- Update function for the game Over state
function gameOverUpdate(updateData)
	DebugRenderer:printText(Vec2(-0.2, 0.6), "GAME OVER")
	DebugRenderer:printText(Vec2(-0.2, 0.4), "Press R to restart the Game!")
	return EventResult.Handled
end
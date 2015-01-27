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
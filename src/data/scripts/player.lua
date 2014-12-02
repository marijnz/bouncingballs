logMessage("using player.lua")

player = GameObjectManager:createGameObject("player")
player.render = player:createRenderComponent()
player.render:setPath("data/models/player.thModel")
player:setPosition(Vec3(0, 0, 0))
player.speed = 0.1
player.update = function (deltaTime)
    -- The direction the player is going to walk this frame
    local direction = Vec3(0.0, 0.0, 0.0)
    if (InputHandler:isPressed(Key.W)) then
        direction = direction + Vec3(-1, -1, 0)
    end
    if (InputHandler:isPressed(Key.S)) then
        direction = direction + Vec3(1, 1, 0)
    end
    if (InputHandler:isPressed(Key.A)) then
        direction = direction + Vec3(1, -1, 0)
    end
    if (InputHandler:isPressed(Key.D)) then
        direction = direction + Vec3(-1, 1, 0)
    end

    -- If a direction is set, walk 
    if (direction.x ~= 0 or direction.y ~= 0) then
        -- Normalize the direction for when two buttons are pressed at the same time
        local normalized = direction:normalized()
        -- Multiply speed by the direction to walk
        player:setPosition(player:getPosition() + Vec3(normalized.x * player.speed, normalized.y * player.speed, normalized.z * player.speed))
    end
end
player.sc = player:createScriptComponent()
player.sc:setUpdateFunction(player.update)

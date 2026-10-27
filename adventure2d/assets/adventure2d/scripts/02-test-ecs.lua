-- ECS (Entity Component System) demo
local world

function joy.load()
    local kkkk = { x = 0.1, y = 0.2 }
    local str = joy.cjson.encode(kkkk)
    console.log(str)

    console.log("test lua ECS:")
    console.log("float:" .. joy.ecs.sizeoffloat())
    console.log("integer:" .. joy.ecs.sizeofinteger())
    console.log("short:" .. joy.ecs.sizeofshort())

    world = joy.ecs.create()
    joy.ecs.define(world, "position_t", 512)
    joy.ecs.define(world, "velocity_t", 512)

    local entity_id = joy.ecs.spawn(world)
    joy.ecs.set(world, entity_id, "position_t", str)
    joy.ecs.set(world, entity_id, "velocity_t",
        joy.cjson.encode({ x = 1.1, y = 1.2 }))

    str = joy.ecs.get(world, entity_id, "position_t", "velocity_t")
    console.log(str)

    -- register system
    joy.ecs.register(world, function(w)
        local entities = joy.ecs.query(w, {"position_t", "velocity_t"})
        for i = 1, #entities do
            local t = joy.ecs.get(w, entities[i], "position_t")
            local t2 = joy.ecs.get(w, entities[i], "velocity_t")
            local position = joy.cjson.decode(t)
            local velocity = joy.cjson.decode(t2)
            position.x = position.x + 1
            position.y = position.y + 1
            velocity.x = velocity.x + 1
            velocity.y = velocity.y + 1
            console.log("entity:" .. entities[i] .. " position:<"
                .. position.x .. "," .. position.y .. ">"
                .. " velocity:<" .. velocity.x .. "," .. velocity.y .. ">")
            joy.ecs.set(w, entities[i], "position_t", joy.cjson.encode(position))
            joy.ecs.set(w, entities[i], "velocity_t", joy.cjson.encode(velocity))
        end
    end)

    joy.event.quit()
end

function joy.update(dt)
    if world then joy.ecs.process(world) end
end

function joy.draw()
end

function joy.quit()
    if world then joy.ecs.destroy(world) end
    console.log("ECS demo exit")
end

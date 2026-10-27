local app = {};

app.grad_size = 50;

app.convert_to_codepoints = function(str)
    local codepoints = {};
    for pos, char in utf8.codes(str) do
        table.insert(codepoints, char);
    end
    return codepoints;
end

local function handle_player_joins(game_world, world, player_joins)
    if not player_joins then
        return;
    end
    for _, player_join in ipairs(player_joins) do
        local body = world2df.create_rigidbody(world, mathx.from_float(1.0), mathx.from_float(1.0));
        local entityid = rigidbody.get_id(body);
        rigidbody.set_position(body, player_join.position_x, player_join.position_y);
        world2df.add_connection(world, player_join.conv, entityid);

        local entity_id = ecs.spawn(app.game_world);
        ecs.set(app.game_world, entity_id, "position", cjson.encode({
            x = mathx.to_float(player_join.position_x),
            y = mathx.to_float(player_join.position_y)
        }));
        ecs.set(app.game_world, entity_id, "size", cjson.encode({
            x = 1.0,
            y = 1.0
        }));
        ecs.set(app.game_world, entity_id, "player", cjson.encode({
            conv = player_join.conv
        }));
    end
end

local function handle_player_leaves(server_k, world, player_leaves)
    if not player_leaves then
        return;
    end
    for _, player_leave in ipairs(player_leaves) do
        local ok, entityid = world2df.get_entity_id(world, player_leave.conv);
        if ok then
            server.remove_connection(server_k, player_leave.conv);
            world2df.remove_connection(world, player_leave.conv);
            world2df.destroy_rigidbody(world, entityid);

            -- deleting renderer
            local entities = ecs.query(app.game_world, {"player"});
            for i = 1, #entities do
                local player_str = ecs.get(app.game_world, entities[i], "player");
                local player = cjson.decode(player_str);
                if player_leave.conv == player.conv then
                    ecs.kill(app.game_world, entities[i]);
                    break;
                end
            end
        end
    end
end

local function handle_player_inputs(world, player_inputs)
    if not player_inputs then
        return;
    end

    core.log(app.ctx, "=========apply inputs===========");
    for _, player_input in ipairs(player_inputs) do
        core.log(app.ctx, "conv:" .. player_input.conv .. " keycode:" .. player_input.keycode);
        world2df.move_player(world, player_input.conv, player_input.keycode);
    end
end

local function handle_creating_emenies(world, creating_emenies)
    if not creating_emenies then
        return;
    end
    for _, creating_emeny in ipairs(creating_emenies) do
        local body = world2df.create_rigidbody(world, creating_emeny.width, creating_emeny.height);
        rigidbody.set_linear_velocity(body, creating_emeny.linear_velocity_x, creating_emeny.linear_velocity_y);
        rigidbody.set_position(body, creating_emeny.position_x, creating_emeny.position_y);
        world2df.add_emeny(world, rigidbody.get_id(body));

        local entity_id = ecs.spawn(app.game_world);
        ecs.set(app.game_world, entity_id, "position", cjson.encode({
            x = mathx.to_float(creating_emeny.position_x),
            y = mathx.to_float(creating_emeny.position_y)
        }));
        ecs.set(app.game_world, entity_id, "size", cjson.encode({
            x = mathx.to_float(creating_emeny.width),
            y = mathx.to_float(creating_emeny.height),
        }));
        ecs.set(app.game_world, entity_id, "emeny", cjson.encode({
            entityid = rigidbody.get_id(body)
        }));
    end
end

app.start = function(ctx)
    local ok, width, height;
    app.ctx = ctx;
    core.log(app.ctx, "server start");
    sdl.set_app_metadata("server2d", "1.0", "com.livnet.server2d");
    app.map_size = {
        x = 8,
        y = 6
    };
    ok, app.window, app.renderer = window.create_with_renderer("server2d", app.map_size.x * 50, app.map_size.y * 50, 32);
    window.set_icon(app.window, "./textures/livnet.bmp");
    graphics.set_logical_presentation(app.renderer, app.map_size.x * 50, app.map_size.y * 50, 1);
    ok, width, height = graphics.get_logical_presentation(app.renderer);
    app.camera = { x = width, y = height };

    local ip = core.get_env(app.ctx, "ip");
    local port = math.tointeger(core.get_env(app.ctx, "port"));
    app.world = world2df.create();
    app.kcpserver = net.kcpserver.create(ip, port);
    app.server = server.create();

    app.fps = profiler.simple_fps.create();

    app.fonts = {};
    app.texts = {};
    app.fonts.simhei_font = graphics.create_font(app.renderer, "fonts/simhei.ttf", app.grad_size * 0.4);
    app.texts.hello_text = graphics.create_text(app.fonts.simhei_font, app.convert_to_codepoints("上"), 255, 255, 255,
        255);
    app.texts.fps_text = graphics.create_text(app.fonts.simhei_font, app.convert_to_codepoints("fps:" .. 1000), 255,
        255, 255, 255);

    -- ecs
    app.game_world = ecs.create();
    ecs.define(app.game_world, "position", 256);
    ecs.define(app.game_world, "velocity", 256);
    ecs.define(app.game_world, "size", 256);
    ecs.define(app.game_world, "player", 256);
    ecs.define(app.game_world, "emeny", 256);

    app.anim1 = animation.create(app.renderer);
    -- add clip
    animation.addclip(app.anim1, "textures/deco/01.png", 0.1, 0, 0, 60, 60);
    animation.addclip(app.anim1, "textures/deco/04.png", 0.1, 0, 0, 60, 60);

    app.tree_anim = animation.create(app.renderer);
    animation.set_position(app.tree_anim, 0, 0);
    animation.set_scale(app.tree_anim, 1, 1);
    animation.set_rotation(app.tree_anim, 0);
    -- add clip
    animation.addclip(app.tree_anim, "textures/trees/tree.png", 0.1, 42, 0, 110, 186);
    animation.addclip(app.tree_anim, "textures/trees/tree.png", 0.1, 234, 0, 110, 186);
    animation.addclip(app.tree_anim, "textures/trees/tree.png", 0.1, 427, 0, 110, 186);
    animation.addclip(app.tree_anim, "textures/trees/tree.png", 0.1, 618, 0, 110, 186);

    app.my_timer = timer.create(66, function(dt, interval)
        -- 没有客户端在
        if server.connections_count(app.server) <= 0 then
            return;
        end
        
        if not server.has_command(app.server) then
            --return;
        end

        -- 收集消息,并执行
        local world_payload = world2df.serialize(app.world);
        local world_checksum = world2df.checksum(app.world);
        local str = server.collect_command(app.server, world_payload, world_checksum);
        -- core.log(app.ctx, str);
        local command = s2c.deserialize(str);
        if s2c.is_command(command.cmd) then
            --local world_checksum = world2df.checksum(app.world);
            if #command.player_leaves > 0 then
                handle_player_leaves(app.server, app.world, command.player_leaves);
            end
            if #command.player_joins > 0 then
                handle_player_joins(app.game_world, app.world, command.player_joins);
            end
            if #command.player_inputs > 0 then
                handle_player_inputs(app.world, command.player_inputs);
            end
            if #command.creating_emenies > 0 then
                handle_creating_emenies(app.world, command.creating_emenies);
            end
            world2df.update_emeny(app.world, mathx.from_float(app.map_size.x), mathx.from_float(app.map_size.y));
        end

        local enemies_count = world2df.enemies_count(app.world);
        if enemies_count >= 10 then
                core.log(app.ctx, "enemies count > 2 " .. enemies_count);
        else
                core.log(app.ctx, "enemy spawn timer:" .. enemies_count);
                local r1 = math.random();
                local r2 = math.random() - 0.5;
                local r3 = math.random(1, app.map_size.y);
                core.log(app.ctx, "r1:" .. r1 .. " r2:" .. r2 .. " r3:" .. r3);
                local width = mathx.from_float(r1);
                local height = width;
                local liner_velocity_x = mathx.from_float(r2);
                local liner_velocity_y = 0;
                local position_x = 0;
                if r2 >= 0 then
                        position_x = 0;
                else
                        position_x = mathx.from_float(app.map_size.y);
                end
                local position_y = mathx.from_float(r3);
                server.create_enemy(app.server, width, height, liner_velocity_x, liner_velocity_y, position_x, position_y);
        end

        server.sync(app.server, function(conv, str)
            net.kcpserver.send(app.kcpserver, conv, str, string.len(str));
        end);
    end);

    -- player move
    ecs.register(app.game_world, function(game_world, dt)
        local entities = ecs.query(game_world, {"position", "player"});
        for i = 1, #entities do
            local position_str = ecs.get(game_world, entities[i], "position");
            local player_str = ecs.get(game_world, entities[i], "player");
            local position = cjson.decode(position_str);
            local player = cjson.decode(player_str);

            core.log(app.ctx, "=============== player move ====================");

            local body = world2df.get_player(app.world, player.conv);
            if body then
                local x, y = rigidbody.get_position(body);
                core.log(app.ctx, "========x:" .. x .. " y:" .. y .. "===========");
                -- position = vec2.lerp(position, {
                --     x = x,
                --     y = y
                -- }, dt * 10);
                position.x = x;
                position.y = y;

                ecs.set(game_world, entities[i], "position", cjson.encode({
                    x = position.x,
                    y = position.y
                }));
            end
        end
    end);

    -- emeny move
    ecs.register(app.game_world, function(game_world, dt)
        local entities = ecs.query(game_world, {"position", "emeny"});
        for i = 1, #entities do
            local position_str = ecs.get(game_world, entities[i], "position");
            local emeny_str = ecs.get(game_world, entities[i], "emeny");
            local position = cjson.decode(position_str);
            local emeny = cjson.decode(emeny_str);

            local body = world2df.get_rigidbody(app.world, emeny.entityid);
            if body then
                local x, y = rigidbody.get_position(body);
                position = vec2.lerp(position, {
                    x = x,
                    y = y
                }, dt * 10);
                ecs.set(game_world, entities[i], "position", cjson.encode({
                    x = position.x,
                    y = position.y
                }));
            end
        end
    end);

    --renderer
    ecs.register(app.game_world, function(game_world, dt)
        local entities = ecs.query(game_world, {"position", "size"});
        for i = 1, #entities do
            local position_str = ecs.get(game_world, entities[i], "position");
            local size_str = ecs.get(game_world, entities[i], "size");
            local position = cjson.decode(position_str);
            local size = cjson.decode(size_str);
            -- core.log(app.ctx, position_str);
            graphics.rectangle(app.renderer, "fill", position.x * 50, position.y * 50, size.x * 50, size.y * 50);
        end
    end);

    app.enemy_spawn_timer = timer.create(3000, function(dt, interval)
        -- local enemies_count = world2df.enemies_count(app.world);
        -- if enemies_count >= 2 then
        --         core.log(app.ctx, "enemies count > 2 " .. enemies_count);
        -- else
        --         core.log(app.ctx, "enemy spawn timer:" .. enemies_count);
        --         local width = mathx.from_float(1);
        --         local height = mathx.from_float(1);
        --         local liner_velocity_x = mathx.from_float(1);
        --         local liner_velocity_y = mathx.from_float(1);
        --         local position_x = mathx.from_float(math.random(5));
        --         local position_y = mathx.from_float(1);
        --         server.create_enemy(app.server, width, height, liner_velocity_x, liner_velocity_y, position_x, position_y);
        -- end
    end);
end

app.event = function(ev)
    if keyboard.is_down(ev, "Q") then
        core.quit(app.ctx);
    elseif keyboard.is_down(ev, "ESCAPE") then
        core.quit(app.ctx);
    end
end

app.update = function(dt)
    net.kcpserver.update(app.kcpserver);
    local result, msg = net.kcpserver.poll(app.kcpserver);
    if result then
        if msg.type == 0 then
            core.log(app.ctx, "connected:" .. msg.conv);
        elseif msg.type == 1 then
            core.error(app.ctx, "============disconnected=========:" .. msg.conv);
            server.handle_player_leave(app.server, msg.conv);
        elseif msg.type == 2 then
            local t = c2s.deserialize(msg.data);
            -- core.log(app.ctx, cjson.encode(t));
            if t.cmd == 2 then
                core.log(app.ctx, "C2S_CMD_READY");
                local str = server.handle_cmd_ready(app.server, msg.conv);
                net.kcpserver.send(app.kcpserver, msg.conv, str, string.len(str));
            elseif t.cmd == 3 then
                core.log(app.ctx, "C2S_CMD_LOADING");
                local str = server.handle_cmd_loading(app.server, msg.conv);
                net.kcpserver.send(app.kcpserver, msg.conv, str, string.len(str));
            elseif t.cmd == 4 then
                core.log(app.ctx, "C2S_CMD_PLAYER_JOIN");
                server.handle_player_join(app.server, msg.conv, t.position_x, t.position_y);
            elseif t.cmd == 5 then
                core.log(app.ctx, "============C2S_CMD_PLAYER_LEAVE===========");
                server.handle_player_leave(app.server, msg.conv);
            elseif t.cmd == 6 then
                -- core.log(app.ctx, "C2S_CMD_PLAYER_INPUT");
                server.handle_player_input(app.server, msg.conv, t.keycode);
            elseif t.cmd == 7 then
                server.handle_cmd_heartbeat(app.server, msg.conv);
            end
        end
    end

    local fps = profiler.simple_fps.update(app.fps);
    animation.update(app.anim1, dt);
    animation.update(app.tree_anim, dt);
    timer.trigger(app.my_timer);
    timer.trigger(app.enemy_spawn_timer);

    graphics.set_color(app.renderer, 0, 0, 0, 255);
    graphics.clear(app.renderer);
    graphics.set_color(app.renderer, 128, 128, 128, 255);
    graphics.gridx(app.renderer, {
        x = -10,
        y = -10
    }, 10, 10, 50);

    ecs.process(app.game_world, dt);
    -- world2df.foreach_body(app.world, function(body)
    --         --core.log(app.ctx, "body:" .. rigidbody.tostring(body));
    --         local x, y = rigidbody.get_position(body);
    --         local width = rigidbody.get_width(body) * 50;
    --         local height = rigidbody.get_height(body) * 50;
    --         --graphics.rectangle(app.renderer, "fill", x * 50, y * 50, width, height);
    -- end);

    local server_frame_id = server.get_frame_id(app.server);
    local str = app.convert_to_codepoints("fps:" .. fps .. " frameid:" .. server_frame_id);
    graphics.update_text(app.texts.fps_text, app.fonts.simhei_font, str, 255, 255, 255, 255);
    graphics.print_text(app.renderer, app.texts.fps_text, 100, 0);

    animation.draw(app.anim1, 0, 0, 0, 0);
    animation.draw(app.tree_anim, 0, 0, 0, 0);
    graphics.present(app.renderer);
end

app.destroy = function()
    ecs.destroy(app.game_world);
    animation.destroy(app.anim1);
    animation.destroy(app.tree_anim);
    timer.destroy(app.enemy_spawn_timer);
    timer.destroy(app.my_timer);
    world2df.destroy(app.world);
    graphics.destroy(app.renderer);
    window.destroy(app.window);
    net.kcpserver.destroy(app.kcpserver);
    server.destroy(app.server);
    profiler.simple_fps.destory(app.fps);
    core.log(app.ctx, "server destroy");
end

return app;

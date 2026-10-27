local app = {};

app.keycodes = {
    NONE = 0x0,
    UP = 0x1,
    DOWN = 0x2,
    LEFT = 0x3,
    RIGHT = 0x4,
    ENTER = 0xD,
    ESC = 0x1B,
    SPACE = 0x20
}

app.convert_to_codepoints = function(str)
    local codepoints = {};
    for pos, char in utf8.codes(str) do
        table.insert(codepoints, char);
    end
    return codepoints;
end

app.grad_size = 50;

app.fonts = {};
app.texts = {};
app.buttons = {};


ecs.setex = function(world, entity_id, typename, json_object)
    ecs.set(world, entity_id, typename, cjson.encode(json_object));
end


local init_ui = function()
    app.fonts.simhei_font = graphics.create_font(app.renderer, "fonts/simhei.ttf", app.grad_size * 0.4);
    app.texts.hello_text = graphics.create_text(app.fonts.simhei_font, app.convert_to_codepoints("上"), 255, 255, 255,
        255);
    app.texts.fps_text = graphics.create_text(app.fonts.simhei_font, app.convert_to_codepoints("fps:"), 255, 255, 255, 255);

    app.texts.ip_port_text = graphics.create_text(app.fonts.simhei_font, app.convert_to_codepoints("ip:" .. app.ip .. " port:" .. app.port), 255, 255, 255, 255);
    --"ip:" .. app.ip .. " port:" .. app.port .. 

    app.buttons.move_up_btn = ui.button.create(app.renderer, app.grad_size, (app.map_size.y - 3) * app.grad_size,
        app.grad_size, app.grad_size);
    ui.button.set_text(app.buttons.move_up_btn, app.fonts.simhei_font, app.convert_to_codepoints("上"), 0, 255, 0, 255);

    app.buttons.move_down_btn = ui.button.create(app.renderer, app.grad_size, (app.map_size.y - 1) * app.grad_size,
        app.grad_size, app.grad_size);
    ui.button.set_text(app.buttons.move_down_btn, app.fonts.simhei_font, app.convert_to_codepoints("下"), 0, 255, 0,
        255);

    app.buttons.move_left_btn = ui.button.create(app.renderer, 0, (app.map_size.y - 2) * app.grad_size, app.grad_size,
        app.grad_size);
    ui.button.set_text(app.buttons.move_left_btn, app.fonts.simhei_font, app.convert_to_codepoints("左"), 0, 255, 0,
        255);

    app.buttons.move_right_btn = ui.button.create(app.renderer, 2 * app.grad_size, (app.map_size.y - 2) * app.grad_size,
        app.grad_size, app.grad_size);
    ui.button.set_text(app.buttons.move_right_btn, app.fonts.simhei_font, app.convert_to_codepoints("右"), 0, 255, 0,
        255);

    app.left_joystick = joystick.create(app.renderer, 20, 20, 40);
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
        ecs.setex(app.game_world, entity_id, "position", {
            x = mathx.to_float(player_join.position_x),
            y = mathx.to_float(player_join.position_y)
        });
        ecs.setex(app.game_world, entity_id, "size", {
            x = 1.0,
            y = 1.0
        });
        ecs.setex(app.game_world, entity_id, "player", {
            conv = player_join.conv
        });
    end
end

local function handle_player_leaves(world, player_leaves)
    if not player_leaves then
        return;
    end
    for _, player_leave in ipairs(player_leaves) do
        local ok, entityid = world2df.get_entity_id(world, player_leave.conv);
        if ok then
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
    local ok, conv = net.kcpclient.get_conv(app.kcpclient);
    for _, player_input in ipairs(player_inputs) do
        core.log(app.ctx, "conv:" .. player_input.conv .. " keycode:" .. player_input.keycode);
        if conv == player_input.conv then
            client.pop_local_input(app.client);
        end
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


local message = function(data)
    local t = s2c.deserialize(data);
    if t.cmd == 3 then
        core.log(app.ctx, cjson.encode(t));
        core.log(app.ctx, "S2C_CMD_LOADING:" .. t.frame_id);
        if t.ok then
            core.log(app.ctx, "loaded");
            if not client.cmd_loading1(app.client, app.world, t.ok, t.data, t.frame_id) then
                return;
            end

            -- 判断添加了玩家,敌人的数量
            world2df.foreach_connection(app.world, function(conv, body_id)
                local body = world2df.get_player(app.world, conv);
                local x, y = rigidbody.get_position(body);
                local width = rigidbody.get_width(body);
                local height = rigidbody.get_height(body);

                local entity_id = ecs.spawn(app.game_world);
                ecs.setex(app.game_world, entity_id, "position", {
                    x = x,
                    y = y
                });
                ecs.setex(app.game_world, entity_id, "size", {
                    x = width,
                    y = height
                });
                ecs.setex(app.game_world, entity_id, "player", {
                    conv = conv
                });
            end);

            local str = client.create_player_join(1.2, 2.3);
            net.kcpclient.send(app.kcpclient, str, string.len(str));

            -- 保存第一帧
            local world_data = world2df.serialize(app.world);
            local world_checksum = world2df.checksum(app.world);
            client.add_world(app.client, t.frame_id, world_data);
            core.log(app.ctx, "cmd_loaded:" .. world_checksum);
        else
            core.log(app.ctx, "loading");
            local str = client.cmd_loading2(app.client, t.ok, t.data);
            if str then
                net.kcpclient.send(app.kcpclient, str, string.len(str));
            end
        end
    elseif t.cmd == 4 then
        --core.log(app.ctx, cjson.encode(t));

        --1.第一步先把当前世界回滚到上一个确认的帧上
        client.rollback(app.client, app.world);

        --2.第二再执行当前到达的帧
        client.set_frameid(app.client, t.frame_id);
        local world_checksum = world2df.checksum(app.world);
        if t.checksum == world_checksum then
            core.log(app.ctx, "S2C_CMD_COMMAND:" .. t.checksum .. ":" .. world_checksum);
        else
            core.error(app.ctx, "S2C_CMD_COMMAND:" .. t.checksum .. ":" .. world_checksum);
        end
        if #t.player_leaves > 0 then
            handle_player_leaves(app.world, t.player_leaves);
        end
        
        if #t.player_joins > 0 then
            handle_player_joins(app.game_world, app.world, t.player_joins);
        end

        if #t.player_inputs > 0 then
            handle_player_inputs(app.world, t.player_inputs);
        end

        if #t.creating_emenies > 0 then
            handle_creating_emenies(app.world, t.creating_emenies);
        end

        -- 敌人操作
        world2df.update_emeny(app.world, mathx.from_float(app.map_size.x), mathx.from_float(app.map_size.y));

        -- 执行完后添加world备份
        local world_data = world2df.serialize(app.world);
        --local world_checksum = world2df.checksum(app.world);
        client.add_world(app.client, t.frame_id, world_data);

        --3.在此基础再继续预测
        client.predict(app.client, app.world);
    end

end

app.start = function(ctx)
    local ok, width, height;
    app.ctx = ctx;
    core.log(app.ctx, "client start");
    sdl.set_app_metadata("server2d", "1.0", "com.livnet.client2d");
    app.map_size = {
        x = 8,
        y = 6
    };
    ok, app.window, app.renderer = window.create_with_renderer("client2d", app.map_size.x * app.grad_size,
        app.map_size.y * app.grad_size, 32);
    window.set_icon(app.window, "./textures/livnet.bmp");
    graphics.set_logical_presentation(app.renderer, app.map_size.x * app.grad_size, app.map_size.y * app.grad_size, 1);
    ok, width, height = graphics.get_logical_presentation(app.renderer);
    app.camera = {
        x = width,
        y = height
    };

    -- ecs
    app.game_world = ecs.create();
    ecs.define(app.game_world, "position", 256);
    ecs.define(app.game_world, "velocity", 256);
    ecs.define(app.game_world, "size", 256);
    ecs.define(app.game_world, "player", 256);
    ecs.define(app.game_world, "emeny", 256);

    local ip = core.get_env(app.ctx, "ip");
    local port = math.tointeger(core.get_env(app.ctx, "port"));
    app.ip = ip;
    app.port = port;
    core.log(app.ctx, "ip:" .. ip .. " port:" .. port);

    app.world = world2df.create();
    app.kcpclient = net.kcpclient.create(ip, port);
    app.client = client.create();
    app.keyboard_keycode = app.keycodes.NONE;
    app.button_keycode = app.keycodes.NONE;

    app.fps = profiler.simple_fps.create();

    init_ui();

    -- player move
    ecs.register(app.game_world, function(game_world, dt)
        local entities = ecs.query(game_world, {"position", "player"});
        for i = 1, #entities do
            local position_str = ecs.get(game_world, entities[i], "position");
            local player_str = ecs.get(game_world, entities[i], "player");
            local position = cjson.decode(position_str);
            local player = cjson.decode(player_str);

            local body = world2df.get_player(app.world, player.conv);
            if body then
                local x, y = rigidbody.get_position(body);
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

    app.my_timer = timer.create(66, function(dt, interval)
        local ok, conv = net.kcpclient.get_conv(app.kcpclient);
        if not ok then
            return;
        end

        if not client.is_ready(app.client) then
            return;
        end
        
        local keycode = app.keycodes.NONE;
        if app.keyboard_keycode == app.keycodes.NONE and app.button_keycode == app.keycodes.NONE then
            return;
        end

        if app.keyboard_keycode ~= app.keycodes.NONE then
            keycode = app.keyboard_keycode;
        elseif app.button_keycode ~= app.keycodes.NONE then
            keycode = app.button_keycode;
        end

        -- 应用本地输入
        client.apply_input(app.client, app.world, conv, keycode);

        -- 同步到服务器
        local str = c2s.serialize_player_input(keycode);
        net.kcpclient.send(app.kcpclient, str, string.len(str));
    end);

    app.heartbeat_timer = timer.create(3000, function(dt, interval)
        local ok, conv = net.kcpclient.get_conv(app.kcpclient);
        if not ok then
            return;
        end
        local str = c2s.serialize_heartbeat();
        net.kcpclient.send(app.kcpclient, str, string.len(str));
    end);
end

app.event = function(event)
    if keyboard.is_down(event, "Q") then
        core.quit(app.ctx);
    elseif keyboard.is_down(event, "ESCAPE") then
        core.quit(app.ctx);
    elseif keyboard.is_down(event, "A") then
        -- core.quit(app.ctx);
        -- kcpclient.send(app.kcpclient, "hello, server", string.len("hello, server"));
    elseif keyboard.is_down(event, "UP") then
        app.keyboard_keycode = app.keycodes.UP;
    elseif keyboard.is_down(event, "RIGHT") then
        app.keyboard_keycode = app.keycodes.RIGHT;
    elseif keyboard.is_down(event, "DOWN") then
        app.keyboard_keycode = app.keycodes.DOWN;
    elseif keyboard.is_down(event, "LEFT") then
        app.keyboard_keycode = app.keycodes.LEFT;
    elseif keyboard.is_down(event, "SPACE") then
        app.keyboard_keycode = app.keycodes.SPACE;
    elseif keyboard.is_keyup(event) then
        app.keyboard_keycode = app.keycodes.NONE;
    end

    joystick.handle_event(app.left_joystick, event);

    ui.button.listen(app.buttons.move_up_btn, event);
    ui.button.listen(app.buttons.move_down_btn, event);
    ui.button.listen(app.buttons.move_left_btn, event);
    ui.button.listen(app.buttons.move_right_btn, event);

    
end

local draw_ui = function()
    ui.button.draw(app.buttons.move_up_btn);
    ui.button.draw(app.buttons.move_down_btn);
    ui.button.draw(app.buttons.move_left_btn);
    ui.button.draw(app.buttons.move_right_btn);
    joystick.draw(app.left_joystick);
end


app.update = function(dt)
    local fps = profiler.simple_fps.update(app.fps);
    net.kcpclient.update(app.kcpclient);
    local result, msg = net.kcpclient.poll(app.kcpclient);
    if result then
        if msg.type == 0 then
            local ok, conv = net.kcpclient.get_conv(app.kcpclient);
            core.log(app.ctx, "kcp eventcb connected:" .. conv);
            local str = c2s.serialize_ready();
            net.kcpclient.send(app.kcpclient, str, string.len(str));
        elseif msg.type == 1 then
            local ok, conv = net.kcpclient.get_conv(app.kcpclient);
            core.log(app.ctx, "disconnected:" .. conv);
        elseif msg.type == 2 then
            -- sdl.log("etype:" .. event.data);
            message(msg.data);
        end
    end

    timer.trigger(app.my_timer);
    timer.trigger(app.heartbeat_timer);

    graphics.set_color(app.renderer, 0, 0, 0, 255);
    graphics.clear(app.renderer);
    graphics.set_color(app.renderer, 128, 128, 128, 255);

    ecs.process(app.game_world, dt);

    local frameid = client.get_frameid(app.client);
    local str = app.convert_to_codepoints("中fps:" .. fps .. " frameid:" .. frameid);
    graphics.update_text(app.texts.fps_text, app.fonts.simhei_font, str, 255, 255, 255, 255);
    graphics.print_text(app.renderer, app.texts.fps_text, 100, 30);
    graphics.print_text(app.renderer, app.texts.ip_port_text, 100, 0);

    -- world2df.foreach_body(app.world, function (body)
    --         --core.log(app.ctx, "body:" .. rigidbody.tostring(body));
    --         local x, y = rigidbody.get_position(body);
    --         local width = rigidbody.get_width(body) * 50;
    --         local height = rigidbody.get_height(body) * 50;
    --         graphics.rectangle(app.renderer, "fill", x * 50, y * 50, width, height);
    -- end);

    draw_ui();

    graphics.present(app.renderer);


    if ui.button.is_clicked(app.buttons.move_up_btn) then
        app.button_keycode = app.keycodes.UP;
    elseif ui.button.is_clicked(app.buttons.move_down_btn) then
        app.button_keycode = app.keycodes.DOWN;
    elseif ui.button.is_clicked(app.buttons.move_left_btn) then
        app.button_keycode = app.keycodes.LEFT;
    elseif ui.button.is_clicked(app.buttons.move_right_btn) then
        app.button_keycode = app.keycodes.RIGHT;
    else
        app.button_keycode = app.keycodes.NONE;
    end

end

app.destroy = function()
    ecs.destroy(app.game_world);
    timer.destroy(app.heartbeat_timer);
    timer.destroy(app.my_timer);
    ui.button.destroy(app.buttons.move_up_btn);
    world2df.destroy(app.world);
    graphics.destroy(app.renderer);
    window.destroy(app.window);
    net.kcpclient.destroy(app.kcpclient);
    client.destroy(app.client);
    profiler.simple_fps.destory(app.fps);
    core.log(app.ctx, "client destroy");
end

return app;

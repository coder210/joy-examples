local livnet = requirex "livnet"
local protocol = requirex "protocol"
local physics = requirex "physics"
local map = require "joy_editor/scripts/map"

local call = livnet.call;
local constant = livnet.constant;

local program = {};

local context = {};
context.window = nil;
context.renderer = nil;
context.kcps = nil;
context.opkey = constant.OPKEY_NONE;
context.requests = {};
context.connections = {};
context.frames = {};
context.frameid = 1;
context.fps = 0;
context.event_queue = livnet.queue.new();


local function handle_cmd_health(request)
        --core.log(program.ctx, "health:" .. request.cn);
        for _, connection in ipairs(context.connections) do
                if connection.conv == request.cn then
                        connection.health = 10;
                        break;
                end
        end
end

local function handle_cmd_ready(conv)
        local connection = context.connections[conv];
        if connection then
                return;
        end
        -- 整个都在第一帧时
        if context.frameid == 1 then
                connection = {
                        health = 10,
                        frameid = context.frameid,
                        conv = conv,
                        ready = true,
                };
                context.connections[conv] = connection;
                local str = protocol.load_scene(conv, "", connection.ready);
                kcpserver.send(context.kcps, conv, str, string.len(str));
                return;
        end

        local frameid = context.frameid - 1;
        local frame = context.frames[frameid];
        local len = string.len(frame);
        connection = {
                health = 10,
                frameid = context.frameid,
                conv = conv,
                ready = false,
                pointer = 1,
                arr = {}
        };
        context.connections[conv] = connection;

        local frag_length = 256;
        if len >= frag_length then
                local count = math.floor(len / frag_length);
                for i = 1, count, 1 do
                        local begin = 1 + (i - 1) * frag_length;
                        local e = i * frag_length;
                        local data = string.sub(frame, begin, e);
                        table.insert(connection.arr, data);
                end
                if len % frag_length ~= 0 then
                        local i = frag_length * count + 1;
                        local data = string.sub(frame, i, len);
                        table.insert(connection.arr, data);
                end
        else
                table.insert(connection.arr, frame);
        end

        if #connection.arr == connection.pointer then
                connection.ready = true;
        end
        local data = connection.arr[connection.pointer];
        connection.pointer = connection.pointer + 1;
        local str = protocol.load_scene(conv, data, connection.ready);
        kcpserver.send(context.kcps, conv, str, string.len(str));
end

local function handle_cmd_load_scene(conv)
        local connection = context.connections[conv];
        if connection and not connection.ready then
                if #connection.arr == connection.pointer then
                        connection.ready = true;
                end
                local data = connection.arr[connection.pointer];
                connection.pointer = connection.pointer + 1;
                local str = protocol.load_scene(conv, data, connection.ready);
                kcpserver.send(context.kcps, conv, str, string.len(str));
        end
end



local function on_recv(conv, data, len)
        local request = cjson.decode(data);
        if request.cmd == constant.CMD_NONE then

        elseif request.cmd == constant.CMD_HEALTH then
                handle_cmd_health(request);
        elseif request.cmd == constant.CMD_READY then
                handle_cmd_ready(conv);
        elseif request.cmd == constant.CMD_LOAD_SCENE then
                handle_cmd_load_scene(conv);
        elseif request.cmd == constant.CMD_PLAYER_JOIN then
                core.log(program.ctx, "CMD_PLAYER_JOIN");
                table.insert(context.requests, request);
        elseif request.cmd == constant.CMD_PLAYER_LEAVE then
                table.insert(context.requests, request);
        elseif request.cmd == constant.CMD_PLAYER_COMMAND then
                table.insert(context.requests, request);
        end
end


local function collect_requests(frame, requests)
        for _, request in ipairs(requests) do
                if request.cmd == constant.CMD_PLAYER_JOIN then
                        local player_join = {};
                        player_join.cn = request.cn;
                        player_join.px = request.px;
                        player_join.py = request.py;
                        table.insert(frame.pjs, player_join);
                elseif request.cmd == constant.CMD_PLAYER_LEAVE then
                        local player_leave = {};
                        player_leave.cn = request.cn;
                        table.insert(frame.pls, player_leave);
                elseif request.cmd == constant.CMD_PLAYER_COMMAND then
                        local player_command = {};
                        player_command.cn = request.cn;
                        player_command.code = request.code;
                        table.insert(frame.pcs, player_command);
                end
        end
end

local function backups_world(frame)
        frame.eid = physics.get_entity_id();
        for _, entity in ipairs(physics.entities) do
                if entity.static == 0 then
                        table.insert(frame.entities, {
                                id = entity.id,
                                type = entity.type,
                                px = entity.position.x,
                                py = entity.position.y,
                                lvx = entity.linear_velocity.x,
                                lvy = entity.linear_velocity.y,
                                rot = entity.rotation,
                                av = entity.angular_velocity,
                                w = entity.width,
                                h = entity.height
                        });
                end
        end

        for conv, entity_id in pairs(physics.players) do
                table.insert(frame.players, {
                        cn = conv,
                        eid = entity_id,
                });
        end
end

local collect_commands = function(ctx, requests)
        local frame = {
                id = ctx.frameid,
                pjs = {}, -- player joins
                pls = {}, -- player leaves
                pcs = {}, -- player commands
                cs = "",
                eid = 0,
                entities = {},
                players = {}
        };
        ctx.frameid = ctx.frameid + 1;
        collect_requests(frame, requests);
        backups_world(frame);
        frame.cs = physics.get_checksum();
        --core.log(program.ctx, "frameid=" .. frame.id .. " cs" .. frame.cs);
        return frame;
end


local handle_player_joins = function(player_joins)
        if not player_joins then
                return;
        end
        for _, player_join in ipairs(player_joins) do
                local player = Player.new();
                player:set_position(player_join.px, player_join.py);
                physics.add_player(player_join.cn, player.entity.id);
        end
end

local handle_player_leaves = function(player_leaves)
        if not player_leaves then
                return;
        end
        for _, player_leave in ipairs(player_leaves) do
                local entity = physics.find_entity(player_leave.cn);
                if entity then
                        kcpserver.offline(context.kcps, player_leave.cn);
                        livnet.world.remove_object(entity.userdata.gid);
                        physics.remove_player(player_leave.cn);
                        context.connections[player_leave.cn] = nil;
                end
        end
end

local function handle_player_cmds(player_cmds)
        if not player_cmds then
                return;
        end
        for _, player_cmd in ipairs(player_cmds) do
                local entity = physics.find_entity(player_cmd.cn);
                --player_command(entity, player_cmd.code);
                local entity_id = entity.userdata.gid;
                local player = livnet.world.get_object(entity_id);
                player:command(player_cmd.code);
        end
end

local function pack_response(frame)
        --core.log(program.ctx, frame);
        local response = {};
        local t = cjson.decode(frame);
        response.cmd = constant.CMD_FRAME;
        response.id = t.id;
        if #t.pjs > 0 then
                response.pjs = t.pjs;
        end
        if #t.pls > 0 then
                response.pls = t.pls;
        end
        if #t.pcs > 0 then
                response.pcs = t.pcs;
        end
        response.cs = t.cs;
        return response;
end

-- target_framid = 6386
-- curr_frameid = 6383
local get_target_frameid = function(curr_frameid)
        local target_frameid = curr_frameid;
        local diff = context.frameid - curr_frameid;
        if diff >= 50 then
                target_frameid = curr_frameid + 9;
        elseif diff >= 10 and diff < 30 then
                target_frameid = curr_frameid + 4;
        elseif diff > 2 and diff < 10 then
                target_frameid = curr_frameid + 2;
        end
        return target_frameid;
end

local function send_t(conv, connection)
        if not connection.ready then
                return;
        end
        if connection.frameid >= context.frameid then
                return;
        end
        local target_frameid = get_target_frameid(connection.frameid);
        for i = connection.frameid, target_frameid, 1 do
                local frame = context.frames[i];
                --core.log(program.ctx, "i:" .. i .. " frame type:" .. type(frame));
                local str = cjson.encode(pack_response(frame));
                kcpserver.send(context.kcps, conv, str, string.len(str));
        end
        connection.frameid = target_frameid + 1;
end

local function check_health(inst, dt)
        -- local requests = context.requests;
        -- livnet.call.invoke(inst, dt, function ()
        --         for _, connection in ipairs(context.connections) do
        --                 if connection.ready then
        --                          if connection.health == 0 then
        --                                 local command = protocol.player_leavex(connection.conv);
        --                                 table.insert(requests, command);
        --                         end
        --                         connection.health = connection.health - 1;
        --                 end
        --         end
        -- end);
end

program.start = function(ctx)
        sdl.log("server");

        
        local ok, width, height;
        sdl.set_app_metadata("server2d", "1.0", "com.livnet.server2d");
        ok, context.window, context.renderer = sdl.create_window_and_renderer("server2d", 800, 600, 32);
        sdl.set_render_logical_presentation(context.renderer, 800, 600, 1);
        assert(ok);
        ok, width, height = sdl.get_render_logical_presentation(context.renderer);
        livnet.camera.size = {
                x = width,
                y = height,
        };

        local ip = core.get_env(ctx, "ip");
        local port = math.tointeger(core.get_env(ctx, "port"));

        context.kcps = kcpserver.create(ip, port);
        context.sprites = sprite_batch.create(1024);

        program.ctx = ctx;
        program.inst = call.new(66);
        program.last_time = sdl.get_ticks();
        program.dt = 0;
        program.check_health_inst = livnet.call.new(3000);
        program.total_time = 0;
        program.frame_count = 0;

        program.simhei_font = font.open("joy_editor/fonts/simhei.ttf", 12);
        program.simhei_font12 = font.open("joy_editor/fonts/simhei.ttf", 12);

        program.text1 = livnet.get_codepoints("1234512345世界1234512345");
        program.text2 = livnet.get_codepoints("开始");
        program.text3 = livnet.get_codepoints("frameid");
        --program.label1 = label.create(context.renderer, program.simhei_font, program.text1);
        --label.set_position(program.label1, 0, 200);

        program.ok_btn = button.create(context.renderer, 150, 100, 100, 50);
        button.set_text(program.ok_btn, program.simhei_font, program.text1);

        program.label2 = label.create(context.renderer, program.simhei_font12, program.text1);

        -- program.combobox1 = combobox.create(0, 200, 100, 30);
        -- combobox.additem(program.combobox1, "1");
        -- combobox.additem(program.combobox1, "2");
        -- combobox.additem(program.combobox1, "3");
        -- combobox.additem(program.combobox1, "4");

        -- program.datagrid1 = datagrid.create(context.renderer, {x=100, y=200, w=500, h=300}, 3, true);
        -- datagrid.setheaders(program.datagrid1, {livnet.get_codepoints("名字"), livnet.get_codepoints("年龄"), livnet.get_codepoints("职业")});
        -- datagrid.addrow(program.datagrid1, {livnet.get_codepoints("Alice Brown"), livnet.get_codepoints("35"), livnet.get_codepoints("Developer")});
        -- datagrid.addrow(program.datagrid1, {livnet.get_codepoints("Jane Smith"), livnet.get_codepoints("25"), livnet.get_codepoints("Designer")});
        -- datagrid.addrow(program.datagrid1, {livnet.get_codepoints("Bob Johnson"), livnet.get_codepoints("40"), livnet.get_codepoints("Manager")});
        -- datagrid.addrow(program.datagrid1, {livnet.get_codepoints("Jane Smith2"), livnet.get_codepoints("35"), livnet.get_codepoints("Developer")});
        -- --local player1 = Player.new(100);
        -- --player1.draw(nil, nil);
        -- program.group = button_group.create();
        -- local radio1 = radio_button.create(50, 50, 30, "Option 1", 0);
        -- local radio2 = radio_button.create(50, 150, 30, "Option 2", 0);
        -- local radio3 = radio_button.create(50, 210, 30, "Option 3", 0);

        -- button_group.add_button(program.group, radio1);
        -- button_group.add_button(program.group, radio2);
        -- button_group.add_button(program.group, radio3);
        -- button_group.set_selected(program.group, 1);


        -- program.checkbox1 = checkbox.create(50, 50, 20, "Option 1");
        -- program.checkbox2 = checkbox.create(50, 80, 20, "Option 2");
        -- program.checkbox3 = checkbox.create(50, 110, 20, "Option 3 (Disabled)");


        local map_size = { x = 9, y = 10 };
        -- local top_wall = prefab.top_wall(map_size.x);
        -- local bottom_wall = prefab.bottom_wall(map_size);
        -- local left_wall = prefab.left_wall(map_size.y);
        -- local right_wall = prefab.right_wall(map_size);
        livnet.load_tilemap(context.renderer, map);
end

program.event = function(event)
        local lua_event = sdl.get_event(event);
        local etype = lua_event.type;
        if etype == livnet.constant.SDL_EVENT_KEY_DOWN then
                if lua_event.key.scancode == livnet.constant.SDL_SCANCODE_Q then
                        core.quit(program.ctx);
                elseif lua_event.key.scancode == livnet.constant.SDL_SCANCODE_ESCAPE then
                        core.quit(program.ctx);
                end
        end

        button.handle_event(program.ok_btn, event);

        -- combobox.handle_event(program.combobox1, event.event, context.renderer);
        -- datagrid.handle_event(program.datagrid1, event.event, context.renderer);
        -- button_group.handle_event(program.group, event.event, context.renderer);

        -- checkbox.handle_event(program.checkbox1, event.event, context.renderer);
        -- checkbox.handle_event(program.checkbox2, event.event, context.renderer);
        -- checkbox.handle_event(program.checkbox3, event.event, context.renderer);

        
        if button.is_clicked(program.ok_btn) then
                core.log(program.ctx, "pressed");
                button.set_text(program.ok_btn, program.simhei_font, program.text1);
        end

end

program.update = function()
        local current_time = sdl.get_ticks();
        program.dt = current_time - program.last_time;

        -- 66ms
        call.invoke(program.inst, program.dt, function(dt, interval)
                if #context.requests > 0 then
                        --core.log(program.ctx, "request len:" .. #context.requests);
                        local frame = collect_commands(context, context.requests);
                        local str = cjson.encode(frame);
                        table.insert(context.frames, frame.id, str);
                        handle_player_joins(frame.pjs);
                        handle_player_leaves(frame.pls);
                        handle_player_cmds(frame.pcs);
                        --physics.move();
                        --physics.collide();
                        context.requests = {};
                end

                for conv, connection in pairs(context.connections) do
                        send_t(conv, connection);
                end
        end);
        
        kcpserver.update(context.kcps);
        local result, msg = kcpserver.poll_message(context.kcps);
        if result then
                if msg.type == 0 then
                elseif msg.type == 2 then
                        on_recv(msg.conv, msg.data, string.len(msg.data));
                end
        end

        check_health(program.check_health_inst, program.dt);

        program.total_time = program.total_time + program.dt;
        if program.total_time >= 1000 then
                context.fps = program.frame_count;
                program.frame_count = 0;
                program.total_time = program.total_time - 1000;
        else
                program.frame_count = program.frame_count + 1;
        end

        livnet.world.update(program.dt);

        program.last_time = current_time;

        sdl.set_render_draw_color(context.renderer, 0, 92, 92, 255);
        sdl.render_clear(context.renderer);
        sdl.set_render_scale(context.renderer, 1, 1);
        --core.log(program.ctx, "len:" .. #world2.objects);

        --sprite_batch.render(context.sprites);
        --livnet.world.render(context.renderer, context.sprites);

        --label.render(program.label1);
        button.render(program.ok_btn);
        
        -- combobox.render(program.combobox1, context.renderer);
        -- datagrid.render(program.datagrid1, context.renderer);
        -- button_group.render(program.group, context.renderer);
        -- checkbox.render(program.checkbox1, context.renderer);
        -- checkbox.render(program.checkbox2, context.renderer);
        -- checkbox.render(program.checkbox3, context.renderer);

        sdl.set_render_draw_color(context.renderer, 0, 0, 0, 255);
        local str = string.format("fps:%d,frameid:%d,conn num:%d", context.fps, context.frameid, #context.connections);

        --local str = context.fps .. "," .. context.frameid;
        --local str = "1234512345世界1234512345";
        label.set_text(program.label2, program.simhei_font, livnet.get_codepoints(str));
        label.render(program.label2);
        -- local cps_str = livnet.get_codepoints(str);
        -- local label2 = label.create(context.renderer, program.simhei_font12, cps_str);
        -- label.render(label2);
        -- label.destroy(label2);
        --sdl.render_debug_text(context.renderer, 0, 0, str);
        --str = "";
        
        for conv, connection in ipairs(context.connections) do
                str = string.format("%s %d:%d", str, conv, connection.frameid);
        end
        --sdl.render_debug_text(context.renderer, 0, 20, str);
        sdl.render_present(context.renderer);
end


program.destroy = function()
        sprite_batch.destroy(context.sprites);
        label.destroy(program.label1);
        button.destroy(program.ok_btn);
        combobox.destroy(program.combobox1);
        datagrid.destroy(program.datagrid1);
        checkbox.destroy(program.checkbox1);
        checkbox.destroy(program.checkbox2);
        checkbox.destroy(program.checkbox3);
        local window = context.window;
        local renderer = context.renderer;
        local kcps = context.kcps;
        kcpserver.destroy(kcps);
        sdl.destroy_renderer(renderer);
        sdl.destroy_window(window);
        core.log(program.ctx, "server service exit");
end


return program;

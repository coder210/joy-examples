-- adventure 游戏客户端
-- 服务端权威移动 + 客户端预测/回滚 + lerp 插值
local win, ren, font
local PPM = 50
local FW, FH = 192, 192
local MOVE_SPEED = 5.0
local FIXED_DT = 1 / 50
local HEART_INTERVAL = 3.0   -- 心跳间隔秒

-- 网络
local client
local connected = false
local joined = false
local my_id
local TEX_PLAYER_PATH = "joy_beat/textures/knights/troops/warrior/warrior_blue.png"
local TEX_ENEMY_PATH = "joy_beat/textures/knights/troops/warrior/warrior_red.png"

-- 实体
local entities = {}
local enetity_list = { player = {}, enemy = {}, wall = {} }
local input = { x = 0, y = 0, attack = false }
local predict_dt = FIXED_DT  -- 预测用 dt，每帧更新

-- ====== 客户端预测 ======
local input_seq = 0
local pending_inputs = {}
local my_ack_seq = 0
local MAX_PENDING = 128
local server_frame_id = 0

-- 摄像机 (lerp)
local camera = { x = 0, y = 0 }
local function to_screen(wx, wy) return (wx-camera.x)*PPM+480, (wy-camera.y)*PPM+360 end

-- 音频
local audio_footstep, audio_bgm

-- ====== 动画状态机（同服务端） ======
local ATK_NAMES = { [0] = "attack_d", [1] = "attack_l", [2] = "attack_r", [3] = "attack_u" }
local ATK_ROWS = { [0] = 4, [1] = 2, [2] = 2, [3] = 6 }
local function is_attack(s)
    return s == "attack_d" or s == "attack_l" or s == "attack_r" or s == "attack_u"
end

local function setup_animator(ren, tex_path)
    local anim = joy.animator.create(ren)
    -- idle (行0)
    joy.animator.add_state(anim, "idle")
    for col = 0, 5 do
        joy.animator.state_add_clip(anim, "idle", tex_path, 0.15, { x = col * FW, y = 0, w = FW, h = FH })
    end
    joy.animator.state_set_loop(anim, "idle", true)
    -- walk (行1)
    joy.animator.add_state(anim, "walk")
    for col = 0, 5 do
        joy.animator.state_add_clip(anim, "walk", tex_path, 0.12, { x = col * FW, y = 1 * FH, w = FW, h = FH })
    end
    joy.animator.state_set_loop(anim, "walk", true)
    -- 4方向攻击
    for d = 0, 3 do
        joy.animator.add_state(anim, ATK_NAMES[d])
        for col = 0, 5 do
            joy.animator.state_add_clip(anim, ATK_NAMES[d], tex_path, 0.08, { x = col * FW, y = ATK_ROWS[d] * FH, w = FW, h = FH })
        end
        joy.animator.state_set_loop(anim, ATK_NAMES[d], false)
        local t = joy.animator.add_transition(anim, ATK_NAMES[d], "idle")
        joy.animator.transition_set_exit_time(anim, t, 0.08 * 6)
    end
    joy.animator.set_default_state(anim, "idle")
    -- idle ↔ walk (speed条件)
    local t = joy.animator.add_transition(anim, "idle", "walk")
    joy.animator.transition_add_cond(anim, t, "speed", 0, 0.1)
    t = joy.animator.add_transition(anim, "walk", "idle")
    joy.animator.transition_add_cond(anim, t, "speed", 1, 0.1)
    -- Any State → 攻击
    for d = 0, 3 do
        t = joy.animator.add_any_transition(anim, ATK_NAMES[d])
        joy.animator.transition_add_cond(anim, t, "atk_trig", 6, 0)
        joy.animator.transition_add_cond(anim, t, "dir", 2, d)
    end
    return anim
end

local function dir_to_idx(rad)
    if rad > -0.75 and rad <= 0.75 then return 2 end
    if rad > 0.75 and rad <= 2.35 then return 3 end
    if rad > 2.35 or rad <= -2.35 then return 1 end
    return 0
end

-- ====== 碰撞（同服务端） ======
local col = joy.collision2df
local fp = joy.fp
local function resolve_collision(e, dx, dy)
    local new_x, new_y = e.pos.x + dx, e.pos.y + dy
    local colliders = {}
    for _, w in ipairs(enetity_list.wall) do table.insert(colliders, w) end
    for _, p in ipairs(enetity_list.player) do if p.id ~= e.id then table.insert(colliders, p) end end
    for _, en in ipairs(enetity_list.enemy) do if en.id ~= e.id then table.insert(colliders, en) end end
    for _, o in ipairs(colliders) do
        local ra = { x = fp.from_float(new_x - e.size.w/2), y = fp.from_float(new_y - e.size.h/2),
                     w = fp.from_float(e.size.w), h = fp.from_float(e.size.h) }
        local rb = { x = fp.from_float(o.pos.x - o.size.w/2), y = fp.from_float(o.pos.y - o.size.h/2),
                     w = fp.from_float(o.size.w), h = fp.from_float(o.size.h) }
        local result = col.rectanglesx(ra, 0, rb, 0)
        if result.ok then
            local nx, ny = fp.to_float(result.contact.normal.x), fp.to_float(result.contact.normal.y)
            local d = fp.to_float(result.contact.depth)
            new_x = new_x + nx * d; new_y = new_y + ny * d
        end
    end
    return new_x - e.pos.x, new_y - e.pos.y
end
local function try_move(e, dx, dy)
    if dx == 0 and dy == 0 then return end
    local rx, ry = resolve_collision(e, dx, dy)
    e.pos.x = e.pos.x + rx; e.pos.y = e.pos.y + ry
end

-- ====== 本地 AI（同服务端逻辑，用于敌人平滑移动）=====
local function dist(x1, y1, x2, y2)
    local dx, dy = x2 - x1, y2 - y1; return math.sqrt(dx * dx + dy * dy)
end
local function update_ai(e, dt)
    local ai = e.ai; if not ai then return end
    ai.timer = ai.timer - dt
    local nearest, min_dist
    for _, p in ipairs(enetity_list.player) do
        local d = dist(e.pos.x, e.pos.y, p.pos.x, p.pos.y)
        if not nearest or d < min_dist then nearest = p; min_dist = d end
    end
    if nearest and min_dist < ai.detect_radius then ai.state = (min_dist < ai.attack_radius) and 2 or 1
    else ai.state = 0 end

    if ai.state == 0 then -- patrol
        if ai.timer <= 0 or dist(e.pos.x, e.pos.y, ai.target_x, ai.target_y) < 0.3 then
            local a = math.random() * 2 * 3.14159; local r = math.random() * ai.patrol_radius
            ai.target_x = ai.center_x + math.cos(a) * r; ai.target_y = ai.center_y + math.sin(a) * r; ai.timer = 2.0 + math.random()
        end
        local dx = ai.target_x - e.pos.x; local dy = ai.target_y - e.pos.y; local d = math.sqrt(dx*dx+dy*dy)
        if d > 0.1 then e.vel.x = (dx/d)*e.speed; e.vel.y = (dy/d)*e.speed; e.dir = math.atan(dy, dx); e.action = "walk"
        else e.vel.x, e.vel.y, e.action = 0, 0, "idle" end
    elseif ai.state == 1 then -- chase
        local dx = nearest.pos.x - e.pos.x; local dy = nearest.pos.y - e.pos.y; local d = math.sqrt(dx*dx+dy*dy)
        if d > 0.1 then e.vel.x = (dx/d)*e.speed; e.vel.y = (dy/d)*e.speed; e.dir = math.atan(dy, dx); e.action = "walk" end
    else -- attack（客户端只做视觉，实际伤害由服务端处理）
        e.vel.x, e.vel.y = 0, 0; local dx = nearest.pos.x - e.pos.x; local dy = nearest.pos.y - e.pos.y; e.dir = math.atan(dy, dx)
        if e.attack_timer <= 0 then
            e.action = "attack"; e.attack_timer = 0.8
        else e.action = "idle" end
    end
end

-- ====== 从 map.lua 创建墙壁（同服务端）=====
local function init_walls()
    local map_data = dofile("joy_beat/tilemap/map.lua")
    if map_data and map_data.layers then
        for _, layer in ipairs(map_data.layers) do
            if layer.name == "collision" and layer.objects then
                for _, obj in ipairs(layer.objects) do
                    if obj.shape == "rectangle" and obj.width > 0 and obj.height > 0 then
                        local e = { id = -math.random(99999), type = "wall",
                            pos = { x = obj.x / PPM + obj.width / PPM / 2, y = obj.y / PPM + obj.height / PPM / 2 },
                            size = { w = obj.width / PPM, h = obj.height / PPM } }
                        entities[e.id] = e; table.insert(enetity_list.wall, e)
                    end
                end; break
            end
        end
    end
end

-- ====== 实体管理 + 全量回滚 ======
-- 为实体创建 render_pos（插值用）
local function ensure_render_pos(e)
    if not e.render_pos then e.render_pos = { x = e.pos.x, y = e.pos.y } end
end

local function handle_command(cmd_frame)
    local fid = cmd_frame.frame_id
    if fid then server_frame_id = fid end

    -- 1. 应用服务器权威状态（所有实体）
    if cmd_frame.entities then
        local new_ids = {}
        for _, s in ipairs(cmd_frame.entities) do
            new_ids[s.id] = true
            local e = entities[s.id]
            if e then
                -- 设置权威位置
                e.pos.x = s.x; e.pos.y = s.y
                e.hp = s.hp; e.max_hp = s.max_hp or s.hp
                e.dir = s.dir or e.dir; e.action = s.action or e.action
                if s.id == my_id then
                    local seq = s.seq or 0
                    if seq > my_ack_seq then
                        for ps in pairs(pending_inputs) do if ps <= seq then pending_inputs[ps] = nil end end
                        my_ack_seq = seq
                    end
                end
            else
                local e = {
                    id = s.id, type = s.type,
                    pos = { x = s.x, y = s.y }, vel = { x = 0, y = 0 },
                    size = { w = s.w or 0.6, h = s.h or 0.6 },
                    hp = s.hp, max_hp = s.max_hp or s.hp,
                    dir = s.dir or 0, action = s.action or "idle",
                }
                if s.type == "player" then
                    e.anim = setup_animator(ren, TEX_PLAYER_PATH)
                    table.insert(enetity_list.player, e)
                    if not my_id then my_id = s.id end
                elseif s.type == "enemy" then
                    e.anim = setup_animator(ren, TEX_ENEMY_PATH)
                    e.ai = { center_x=s.x, center_y=s.y, patrol_radius=4.0, detect_radius=15.0, attack_radius=1.5, state=0, timer=0, target_x=s.x, target_y=s.y }
                    e.speed = 2.0; e.attack_timer = 0
                    table.insert(enetity_list.enemy, e)
                end
                ensure_render_pos(e)
                entities[s.id] = e
            end
            if e then ensure_render_pos(e) end
        end
        -- 移除已不存在的实体
        for id, e in pairs(entities) do
            if not new_ids[id] and e.type ~= "wall" then
                if e.type == "player" then for i=#enetity_list.player,1,-1 do if enetity_list.player[i].id==id then table.remove(enetity_list.player,i);break end end
                elseif e.type == "enemy" then for i=#enetity_list.enemy,1,-1 do if enetity_list.enemy[i].id==id then table.remove(enetity_list.enemy,i);break end end end
                entities[id] = nil
            end
        end
    end

    -- 2. 重放未确认的本地输入（用保存的 dt）
    local me = my_id and entities[my_id]
    if me then
        local seqs = {}
        for seq in pairs(pending_inputs) do table.insert(seqs, seq) end
        table.sort(seqs)
        for _, seq in ipairs(seqs) do
            local inp = pending_inputs[seq]
            local replay_dt = inp.dt or FIXED_DT  -- 回退到 FIXED_DT
            if inp.input_x ~= 0 or inp.input_y ~= 0 then
                local len = math.sqrt(inp.input_x*inp.input_x+inp.input_y*inp.input_y)
                me.dir = math.atan(inp.input_y, inp.input_x)
                try_move(me, (inp.input_x/len)*MOVE_SPEED*replay_dt, (inp.input_y/len)*MOVE_SPEED*replay_dt)
            end
            if inp.attack and me.anim and not in_attack(me) then
                me.action = "attack"
                joy.animator.set_trigger(me.anim, "atk_trig")
            end
        end
    end
end

local function in_attack(e)
    return e.anim and is_attack(joy.animator.current_state(e.anim))
end

-- ====== 网络 ======
local heart_timer = 0
local function process_network()
    if not client then return end
    local nc = joy.net.netclient
    nc.update(client)

    if not connected then
        local r = nc.poll(client)
        if r.ok and r.msg then
            if r.msg.type == 1 then
                connected = true
                console.log("connected, sending join...")
                nc.send(client, joy.msgpack.pack({cmd=1, player_join={position_x=10, position_y=5}}))
            end
        end
        return
    end

    -- 接收 S2C 消息
    while true do
        local r = nc.poll(client)
        if not r.ok then break end
        if r.msg.type == 3 then
            local ok, data = pcall(joy.msgpack.unpack, r.msg.data)
            if ok and data then
                if data.cmd == 2 and data.command then
                    local cf = data.command
                    -- 转换 S2CEntity 格式
                    local converted = {frame_id = cf.frame_id, entities = {}}
                    if cf.entities then
                        for _, s in ipairs(cf.entities) do
                            table.insert(converted.entities, {
                                id=s.id, type=(s.type==1) and "player" or "enemy",
                                x=s.position_x, y=s.position_y,
                                w=s.width or 0.6, h=s.height or 0.6,
                                hp=s.hp, max_hp=s.max_hp or s.hp,
                                dir=s.dir or 0, action=s.action or "idle",
                                seq=s.seq or 0
                            })
                        end
                    end
                    -- 处理玩家加入/离开
                    if cf.player_joins then
                        for _, j in ipairs(cf.player_joins) do
                            console.log("player join: conv=" .. tostring(j.conv))
                        end
                    end
                    -- 全量回滚 + 预测重放
                    handle_command(converted)
                    if not joined then
                        joined = true
                        console.log("joined, frame=" .. (cf.frame_id or "?") .. " entities=" .. #converted.entities)
                    end
                end
            end
        elseif r.msg.type == 2 then
            console.log("disconnected")
            connected = false; joined = false
            enetity_list.player = {}; enetity_list.enemy = {}; entities = {}
        end
    end

    -- 发送输入 + 心跳
    if joined then
        local me = my_id and entities[my_id]
        if me then
            local att = input.attack; input.attack = false
            -- 本地预测：立即应用输入
            if att and me.anim and not in_attack(me) then
                me.action = "attack"
                joy.animator.set_trigger(me.anim, "atk_trig")
            end
            -- 记录待确认输入（带上实际 dt）
            input_seq = input_seq + 1
            pending_inputs[input_seq] = { input_x=input.x, input_y=input.y, attack=att, dt=predict_dt }
            if input_seq - my_ack_seq > MAX_PENDING then
                for seq in pairs(pending_inputs) do
                    if seq < input_seq - MAX_PENDING then pending_inputs[seq] = nil end
                end
            end
            -- 本地移动预测（攻击+移动可同时，匹配服务端行为）
            if input.x ~= 0 or input.y ~= 0 then
                local len = math.sqrt(input.x*input.x+input.y*input.y)
                me.dir = math.atan(input.y, input.x)
                try_move(me, (input.x/len)*MOVE_SPEED*predict_dt, (input.y/len)*MOVE_SPEED*predict_dt)
            end
            local k = 0
            if input.y < 0 then k = k | 1 end
            if input.y > 0 then k = k | 2 end
            if input.x < 0 then k = k | 4 end
            if input.x > 0 then k = k | 8 end
            if att then k = k | 16 end
            -- C2SPlayerInput: 只发 keycode，位置由服务器计算
            nc.send(client, joy.msgpack.pack({cmd=3, player_input={keycode=k, seq=input_seq}}))
        end
    end
end

-- ====== 渲染 ======
local function draw_sprite(e)
    local px = (e.render_pos and e.render_pos.x) or e.pos.x
    local py = (e.render_pos and e.render_pos.y) or e.pos.y
    if not e.anim then
        local sx, sy = to_screen(px, py)
        local sw, sh = e.size.w * PPM, e.size.h * PPM
        local r, g = (e.type=="player") and 80 or 220, (e.type=="player") and 130 or 80
        joy.r2d.set_color(ren, r, g, (e.type=="player") and 255 or 80, 255)
        joy.r2d.rectangle(ren, "fill", sx-sw/2, sy-sh/2, sw, sh)
    else
        local sx, sy = to_screen(px, py)
        joy.animator.draw(e.anim, sx - FW/2, sy - FH/2)
    end
    if e.max_hp and e.hp then
        local sx, sy = to_screen(px, py)
        local bw, ratio = 40, e.hp/e.max_hp
        local hp_color = (ratio > 0.6) and {60,200,60,255} or (ratio > 0.3) and {200,200,60,255} or {200,60,60,255}
        joy.r2d.set_color(ren, 60,60,60,200); joy.r2d.rectangle(ren, "fill", sx-bw/2, sy-FH/2-10, bw, 4)
        joy.r2d.set_color(ren, hp_color[1],hp_color[2],hp_color[3],hp_color[4]); joy.r2d.rectangle(ren, "fill", sx-bw/2, sy-FH/2-10, bw*ratio, 4)
    end
end

local tilemap
local function init_tilemap()
    local ok, tm = pcall(joy.tilemap.load_file, "joy_beat/tilemap/map.lua")
    if ok and tm then tilemap = tm; joy.tilemap.render_init_base(ren, "joy_beat/") end
end

function joy.load()
    win = joy.window.create("adventure-client", 400, 300, 0)
    ren = joy.r2d.create(win)
    joy.r2d.set_logical_presentation(ren, 960, 720, 2)
    font = joy.r2d.create_font(ren, "joy_beat/fonts/simhei.ttf", 16)
    init_tilemap(); init_walls()

    local nc = joy.net.netclient
    local ok, c = pcall(nc.create, "kcp", "127.0.0.1", 10000)
    if ok and c then
        client = c
        set_env("client", c)
        console.log("connecting to kcp://127.0.0.1:10000 ...")
    else
        console.log("failed to create client")
    end
    -- 音频
    local ok_a, foot = pcall(joy.audio.create, "joy_beat/audio/ak47.wav")
    if ok_a and foot then audio_footstep = foot end
    local ok_b, bgm = pcall(joy.audio.create, "joy_beat/audio/bgm.wav")
    if ok_b and bgm then audio_bgm = bgm; pcall(joy.audio.play_loop, bgm) end
    console.log("WASD=move SPACE=attack Q=quit")
end

function joy.hotfix()
    client = get_env("client")
    connected = get_env("connected") or false
    joined = get_env("joined") or false
    my_id = get_env("my_id")
end

-- FPS 计数
local fps_counter, fps_timer, fps_display = 0, 0, 0
local footstep_timer = 0

function joy.update(dt)
    predict_dt = dt  -- 用实际帧时间做预测，与服务端 50Hz 解耦
    process_network()

    -- 心跳
    heart_timer = heart_timer + dt
    if joined and heart_timer >= HEART_INTERVAL then
        heart_timer = 0
        if client then
            local nc = joy.net.netclient
            nc.send(client, joy.msgpack.pack({cmd=4, player_heart={}}))
        end
    end

    if joined then
        -- 敌人位置由服务端权威同步，客户端不做 AI 预测（避免不同步）

        -- 动画更新
        for _, list in pairs(enetity_list) do
            for _, e in ipairs(list) do
                if e.anim then
                    local di = dir_to_idx(e.dir)
                    local spd = (e.action == "walk") and 1 or 0
                    joy.animator.set_float(e.anim, "speed", spd)
                    joy.animator.set_float(e.anim, "dir", di)
                    joy.animator.set_flip_h(e.anim, di == 1)
                    local cur = joy.animator.current_state(e.anim)
                    if e.action == "attack" and not is_attack(cur) then
                        joy.animator.set_trigger(e.anim, "atk_trig")
                    end
                    joy.animator.update(e.anim, dt)
                    local after = joy.animator.current_state(e.anim)
                    e.action = is_attack(after) and "attack" or after
                end
            end
        end

        -- 脚步声（行走时）
        if audio_footstep then
            local me = my_id and entities[my_id]
            if me and me.action == "walk" then
                footstep_timer = footstep_timer + dt
                if footstep_timer >= 0.3 then
                    footstep_timer = 0
                    joy.audio.play(audio_footstep)
                end
            else
                footstep_timer = 0
            end
        end

        -- 位置插值 (lerp) — 渲染位置平滑过渡
        for _, list in pairs(enetity_list) do
            for _, e in ipairs(list) do
                if e.render_pos then
                    local lerp = math.min(dt * 10, 1)
                    e.render_pos.x = e.render_pos.x + (e.pos.x - e.render_pos.x) * lerp
                    e.render_pos.y = e.render_pos.y + (e.pos.y - e.render_pos.y) * lerp
                end
            end
        end
    end

    -- 摄像机跟随自己
    local me = my_id and entities[my_id]
    local target = (me and me.render_pos) or (me and me.pos) or {x=0, y=0}
    local lerp = math.min(dt * 5, 1)
    camera.x = camera.x + (target.x - camera.x) * lerp
    camera.y = camera.y + (target.y - camera.y) * lerp

    -- FPS
    fps_counter = fps_counter + 1; fps_timer = fps_timer + dt
    if fps_timer >= 1 then fps_display = fps_counter; fps_counter = 0; fps_timer = fps_timer - 1 end
end

function joy.draw()
    joy.r2d.begin(ren, 30, 35, 45, 255)
    local ms = joy.sdl.get_ticks()
    if tilemap then joy.tilemap.render_all(tilemap, camera.x*PPM-480, camera.y*PPM-360, ms) end

    -- 绘制实体（使用 render_pos 插值位置）
    for _, e in ipairs(enetity_list.enemy) do draw_sprite(e) end
    for _, e in ipairs(enetity_list.player) do draw_sprite(e) end

    -- 调试层
    local status = "disconnected"
    if connected and not joined then status = "loading..."
    elseif joined then status = ("frame="..server_frame_id.." pending="..#pending_inputs) end
    joy.r2d.set_color(ren, 40, 40, 50, 200)
    joy.r2d.rectangle(ren, "fill", 10, 10, 300, 55)
    joy.r2d.set_color(ren, 200, 200, 200, 255)
    joy.r2d.print(ren, font, status, 15, 15, 200, 200, 200, 255)
    joy.r2d.print(ren, font, "FPS:"..fps_display.." players:"..#enetity_list.player.." enemies:"..#enetity_list.enemy, 15, 35, 180, 180, 200, 255)
    joy.r2d.set_color(ren, 150, 150, 150, 255)
    joy.r2d.print(ren, font, "WASD=move SPACE=attack Q=quit", 15, 695, 150, 150, 150, 255)
    joy.r2d.end_frame(ren)
end

function joy.keypressed(key)
    if key == "q" or key == "escape" then joy.event.quit() end
    if key == "w" then input.y = -1 elseif key == "s" then input.y = 1
    elseif key == "a" then input.x = -1 elseif key == "d" then input.x = 1
    elseif key == "space" then input.attack = true end
end
function joy.keyreleased(key)
    if key == "w" and input.y == -1 then input.y = 0
    elseif key == "s" and input.y == 1 then input.y = 0
    elseif key == "a" and input.x == -1 then input.x = 0
    elseif key == "d" and input.x == 1 then input.x = 0 end
end

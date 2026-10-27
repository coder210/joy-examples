-- adventure 游戏（tilemap地图 + 精灵动画 + 网络同步）
-- 碰撞推出 + 帧同步命令历史 + AI攻击对齐
local win, ren, font
local PPM = 50
local FW, FH = 192, 192
local FRAMES = 6
local MOVE_SPEED = 5.0
local FIXED_DT = 1 / 50
local accumulator = 0
local SERVER_TICK = 1 / 20
local net_timer = 0
local frame_id = 0
local worlds = {}       -- frame_id → world snapshot
local MAX_HISTORY = 1000

local server
local server_ip = "127.0.0.1"
local server_port = 10000
local conv_to_player = {}
local player_joins = {}
local player_leaves = {}
local player_inputs = {}


local tilemap
local TEX_PLAYER_PATH = "joy_beat/textures/knights/troops/warrior/warrior_blue.png"
local TEX_ENEMY_PATH = "joy_beat/textures/knights/troops/warrior/warrior_red.png"

local entities = {}
local entity_list = { player = {}, enemy = {}, wall = {} }
local next_id = 1
local function new_id() local id = next_id; next_id = next_id + 1; return id end

local function rect_overlap(a, b)
    return a.x < b.x + b.w and a.x + a.w > b.x and a.y < b.y + b.h and a.y + a.h > b.y
end
local function dist(x1, y1, x2, y2)
    local dx, dy = x2 - x1, y2 - y1; return math.sqrt(dx * dx + dy * dy)
end

-- ====== 创建 janimator 动画状态机 ======
local ATK_NAMES = { [0] = "attack_d", [1] = "attack_l", [2] = "attack_r", [3] = "attack_u" }
local ATK_ROWS = { [0] = 4, [1] = 2, [2] = 2, [3] = 6 }

local function is_attack(s)
    return s == "attack_d" or s == "attack_l" or s == "attack_r" or s == "attack_u"
end

local function setup_animator(ren, tex)
    if not tex then return nil end
    local anim = joy.animator.create(ren)
    joy.animator.set_float(anim, "speed", 0)
    joy.animator.set_float(anim, "dir", 0)

    -- idle (行0)
    joy.animator.add_state(anim, "idle")
    for col = 0, FRAMES - 1 do
        joy.animator.state_add_clip(anim, "idle", tex, 0.15, { x = col * FW, y = 0, w = FW, h = FH })
    end
    joy.animator.state_set_loop(anim, "idle", true)

    -- walk (行1)
    joy.animator.add_state(anim, "walk")
    for col = 0, FRAMES - 1 do
        joy.animator.state_add_clip(anim, "walk", tex, 0.12, { x = col * FW, y = 1 * FH, w = FW, h = FH })
    end
    joy.animator.state_set_loop(anim, "walk", true)

    -- 4方向攻击 (非循环+exit_time)
    for d = 0, 3 do
        joy.animator.add_state(anim, ATK_NAMES[d])
        for col = 0, FRAMES - 1 do
            joy.animator.state_add_clip(anim, ATK_NAMES[d], tex, 0.08, { x = col * FW, y = ATK_ROWS[d] * FH, w = FW, h = FH })
        end
        joy.animator.state_set_loop(anim, ATK_NAMES[d], false)
        local t = joy.animator.add_transition(anim, ATK_NAMES[d], "idle")
        joy.animator.transition_set_exit_time(anim, t, 0.08 * FRAMES)
    end

    joy.animator.set_default_state(anim, "idle")

    -- idle ↔ walk (speed条件)
    local t = joy.animator.add_transition(anim, "idle", "walk")
    joy.animator.transition_add_cond(anim, t, "speed", 0, 0.1)
    t = joy.animator.add_transition(anim, "walk", "idle")
    joy.animator.transition_add_cond(anim, t, "speed", 1, 0.1)

    -- Any State → 攻击 (trigger + dir)
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

function create_player(px, py, conv)
    local e = { id = new_id(), type = "player", conv = conv or 0,
        hp = 10, max_hp = 10, pos = { x = px, y = py }, vel = { x = 0, y = 0 },
        size = { w = 0.6, h = 0.6 }, dir = 0, action = "idle", attack_timer = 0, speed = MOVE_SPEED, keys = 0 }
    entities[e.id] = e; table.insert(entity_list.player, e)
    e.anim = setup_animator(ren, TEX_PLAYER_PATH)
    if conv then conv_to_player[conv] = e.id end
    return e
end

function create_enemy(px, py)
    local e = { id = new_id(), type = "enemy", hp = 5, max_hp = 5,
        pos = { x = px, y = py }, vel = { x = 0, y = 0 }, size = { w = 0.6, h = 0.6 },
        dir = 0, action = "idle", attack_timer = 0, speed = 2.0,
        ai = { center_x = px, center_y = py, patrol_radius = 4.0, detect_radius = 15.0, attack_radius = 1.5,
               state = 0, timer = 0, target_x = px, target_y = py } }
    entities[e.id] = e; table.insert(entity_list.enemy, e)
    e.anim = setup_animator(ren, TEX_ENEMY_PATH)
    return e
end

function create_wall(wx, wy, ww, wh)
    local e = { id = new_id(), type = "wall", pos = { x = wx, y = wy }, size = { w = ww, h = wh } }
    entities[e.id] = e; table.insert(entity_list.wall, e); return e
end

local attack_rays = {}
local function spawn_attack_ray(owner, dir_rad)
    local ox = math.cos(dir_rad) * 0.7; local oy = math.sin(dir_rad) * 0.7
    table.insert(attack_rays, { x = owner.pos.x + ox, y = owner.pos.y + oy, w = 0.8, h = 0.4, lifetime = 0.15, owner = owner, hit = {} })
end

-- ====== 碰撞推出 (jcollision.c SAT) ======
local col = joy.collision2df
local fp = joy.fp

local function resolve_collision(e, dx, dy)
    local new_x, new_y = e.pos.x + dx, e.pos.y + dy
    local colliders = {}
    for _, w in ipairs(entity_list.wall) do table.insert(colliders, w) end
    for _, p in ipairs(entity_list.player) do if p.id ~= e.id then table.insert(colliders, p) end end
    for _, en in ipairs(entity_list.enemy) do if en.id ~= e.id then table.insert(colliders, en) end end

    for _, o in ipairs(colliders) do
        -- rectanglef_t 的 x,y 要求左上角坐标, 需要从中心转换
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

-- ====== 动画更新 (janimator) ======
local function update_sprite(e, dt)
    if not e.anim then return end
    local di = dir_to_idx(e.dir)
    local spd = (e.action == "walk") and 1 or 0
    joy.animator.set_float(e.anim, "speed", spd)
    joy.animator.set_float(e.anim, "dir", di)
    joy.animator.set_flip_h(e.anim, di == 1)

    -- 想在攻击且动画机当前不在攻击态 → 触发 attack transition
    local cur = joy.animator.current_state(e.anim)
    if e.action == "attack" and not is_attack(cur) then
        joy.animator.set_trigger(e.anim, "atk_trig")
    end

    joy.animator.update(e.anim, dt)

    -- 同步 e.action ← animator current state (animator 是最终状态)
    local after = joy.animator.current_state(e.anim)
    if is_attack(after) then
        e.action = "attack"
    else
        e.action = after  -- "idle" / "walk"
    end
end

-- ====== AI ======
local function update_ai(e, dt)
    local ai = e.ai; if not ai then return end
    ai.timer = ai.timer - dt
    local nearest, min_dist
    for _, p in ipairs(entity_list.player) do
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
    else -- attack
        e.vel.x, e.vel.y = 0, 0; local dx = nearest.pos.x - e.pos.x; local dy = nearest.pos.y - e.pos.y; e.dir = math.atan(dy, dx)
        if e.attack_timer <= 0 then
            e.action = "attack"; spawn_attack_ray(e, e.dir); e.attack_timer = 0.8
        else e.action = "idle" end
    end
end

-- ====== 从 keycode 计算速度/方向 ======
local function keycode_to_input(keys)
    local kx, ky, att = 0, 0, false
    if keys & 1 ~= 0 then ky = -1 end
    if keys & 2 ~= 0 then ky = 1 end
    if keys & 4 ~= 0 then kx = -1 end
    if keys & 8 ~= 0 then kx = 1 end
    if keys & 16 ~= 0 then att = true end
    return kx, ky, att
end

-- ====== 固定更新（权威服务器） ======
local function fixed_update(dt)
    -- 处理玩家加入/离开
    for _, pid in ipairs(player_joins) do if entities[pid] then entities[pid].keys = 0 end end
    for _, pid in ipairs(player_leaves) do
        local p = entities[pid]
        if p then p.hp = 0; for conv, pid2 in pairs(conv_to_player) do if pid2 == pid then conv_to_player[conv] = nil; break end end end
    end
    -- 应用客户端输入
    for _, entry in ipairs(player_inputs) do
        local p = entities[entry.pid]
        if p then p.keys = entry.keys; p.inp_seq = entry.seq end
    end
    player_joins, player_leaves, player_inputs = {}, {}, {}

    -- 所有玩家：从 keycode 权威计算移动
    for _, p in ipairs(entity_list.player) do
        if p.attack_timer and p.attack_timer > 0 then p.attack_timer = p.attack_timer - dt end
        if p.keys then
            local kx, ky, att = keycode_to_input(p.keys)
            if att and p.attack_timer <= 0 then
                spawn_attack_ray(p, p.dir); p.attack_timer = 0.5
            end
            if kx ~= 0 or ky ~= 0 then
                local len = math.sqrt(kx*kx+ky*ky)
                p.vel.x = (kx/len)*p.speed; p.vel.y = (ky/len)*p.speed
                p.dir = math.atan(ky, kx)
            else
                p.vel.x, p.vel.y = 0, 0
            end
        end
    end

    -- 敌人 AI + 攻击计时
    for _, e in ipairs(entity_list.enemy) do
        if e.attack_timer and e.attack_timer > 0 then e.attack_timer = e.attack_timer - dt end
        update_ai(e, dt)
    end

    -- 所有实体移动 + 碰撞（权威）
    for _, list in pairs(entity_list) do
        for _, e in ipairs(list) do
            if e.vel then try_move(e, e.vel.x * FIXED_DT, e.vel.y * FIXED_DT) end
        end
    end

    -- 精灵帧更新
    for _, list in pairs(entity_list) do for _, e in ipairs(list) do if e.type ~= "wall" then update_sprite(e, dt) end end end

    -- 攻击射线
    local new_rays = {}
    for _, ray in ipairs(attack_rays) do
        ray.lifetime = ray.lifetime - dt
        if ray.lifetime > 0 then
            local targets = (ray.owner.type == "player") and entity_list.enemy or entity_list.player
            for _, t in ipairs(targets) do
                if not ray.hit[t.id] and rect_overlap({x=ray.x,y=ray.y,w=ray.w,h=ray.h},{x=t.pos.x,y=t.pos.y,w=t.size.w,h=t.size.h}) then
                    t.hp = t.hp - 1; ray.hit[t.id] = true
                end
            end
            table.insert(new_rays, ray)
        end
    end
    attack_rays = new_rays

    -- 清理死亡敌人
    local alive_enemies = {}
    for _, e in ipairs(entity_list.enemy) do
        if not e.hp or e.hp > 0 then table.insert(alive_enemies, e) else entities[e.id] = nil end
    end
    entity_list.enemy = alive_enemies
end

-- ====== 帧同步 ======
-- 收集当前帧的命令（玩家输入/加入/离开）用于 S2CCommand
local function collect_command_frame()
    local joins, inputs = {}, {}
    for conv, pid in pairs(conv_to_player) do
        local p = entities[pid]
        table.insert(joins, {conv=conv, position_x=(p and p.pos.x or 0), position_y=(p and p.pos.y or 0)})
    end
    return joins
end

function save_world_snapshot()
    local ents = {}
    for _, list in pairs(entity_list) do
        for _, e in ipairs(list) do
            if e.type ~= "wall" then
                local entity = {
                    id = e.id,
                    type = (e.type == "player") and 1 or 2,
                    position_x = e.pos.x, position_y = e.pos.y,
                    width = e.size.w, height = e.size.h,
                    hp = e.hp or 0,
                    dir = e.dir or 0, action = e.action or "idle",
                    seq = e.inp_seq or 0, max_hp = e.max_hp or e.hp
                }
                if e.type == "player" then entity.player_conv = e.conv end
                table.insert(ents, entity)
            end
        end
    end
    worlds[frame_id] = ents
    if frame_id > MAX_HISTORY and worlds[frame_id - MAX_HISTORY] then worlds[frame_id - MAX_HISTORY] = nil end
end

local function get_target_frame(conn_frameid, current_fid)
    local diff = current_fid - conn_frameid
    if diff >= 50 then return conn_frameid + 9
    elseif diff >= 30 then return conn_frameid
    elseif diff >= 10 then return conn_frameid + 4
    elseif diff > 2 then return conn_frameid + 2
    else return conn_frameid end
end

-- S2C_CMD_COMMAND = 2
function pack_world_state()
    local ents = {}
    local joins = collect_command_frame()
    local leaves = {}
    for _, pid in ipairs(player_leaves) do
        table.insert(leaves, {conv=pid})
    end
    local inputs_table = {}
    for _, p in ipairs(entity_list.player) do
        if p.keys and p.conv then
            table.insert(inputs_table, {conv=p.conv, keycode=p.keys})
        end
    end
    for _, list in pairs(entity_list) do
        for _, e in ipairs(list) do
            if e.type ~= "wall" then
                local entity = {
                    id = e.id,
                    type = (e.type == "player") and 1 or 2,
                    position_x = e.pos.x, position_y = e.pos.y,
                    width = e.size.w, height = e.size.h,
                    hp = e.hp or 0, max_hp = e.max_hp or e.hp,
                    dir = e.dir or 0, action = e.action or "idle",
                    seq = e.inp_seq or 0
                }
                if e.type == "player" then entity.player_conv = e.conv end
                table.insert(ents, entity)
            end
        end
    end
    return joy.msgpack.pack({cmd=2, command={
        frame_id = frame_id,
        player_joins = joins,
        player_leaves = leaves,
        player_inputs = inputs_table,
        entities = ents
    }})
end

-- ====== 网络 ======
local net = joy.net.netserver
local function process_network()
    if not server then return end
    net.update(server)
    while true do
        local r = net.poll(server)
        if not r.ok then break end
        if r.msg.type == 1 then  -- connected
            local px = 10 + math.random() * 8; local py = 2 + math.random() * 10
            create_player(px, py, r.msg.conv)
            net.send(server, r.msg.conv, pack_world_state())
        elseif r.msg.type == 2 then  -- disconnected
            local pid = conv_to_player[r.msg.conv]
            if pid then table.insert(player_leaves, pid); conv_to_player[r.msg.conv] = nil end
        elseif r.msg.type == 3 then  -- message
            local ok, cmd = pcall(joy.msgpack.unpack, r.msg.data)
            if ok and cmd then
                -- C2S 协议: cmd=1=PLAYER_JOIN, 2=PLAYER_LEAVE, 3=PLAYER_INPUT, 4=PLAYER_HEART
                if cmd.cmd == 1 then  -- C2SPlayerJoin
                    local join = cmd.player_join or {}
                    local px = join.position_x or (10 + math.random() * 8)
                    local py = join.position_y or (2 + math.random() * 10)
                    if not conv_to_player[r.msg.conv] then create_player(px, py, r.msg.conv) end
                elseif cmd.cmd == 3 then  -- C2SPlayerInput (权威: 只存 keycode, 位置由服务器计算)
                    local inp = cmd.player_input or {}
                    local pid = conv_to_player[r.msg.conv]
                    if pid then
                        local p = entities[pid]
                        if p then
                            p.keys = inp.keycode or 0
                            p.inp_seq = inp.seq or p.inp_seq or 0
                        end
                    end
                elseif cmd.cmd == 4 then  -- C2SPlayerHeart
                    -- 心跳，保持连接活跃
                end
            end
        end
    end
end

-- ====== 渲染 ======
local camera = { x = 0, y = 0 }
local function to_screen(wx, wy) return (wx-camera.x)*PPM+480, (wy-camera.y)*PPM+360 end

function joy.load()
    win = joy.window.create("adventure", 400, 300, 0)
    ren = joy.r2d.create(win)
    joy.r2d.set_logical_presentation(ren, 960, 720, 2)
    font = joy.r2d.create_font(ren, "joy_beat/fonts/simhei.ttf", 16)

    local ok, tm = pcall(joy.tilemap.load_file, "joy_beat/tilemap/map.lua")
    if ok and tm then tilemap = tm; joy.tilemap.render_init_base(ren, "joy_beat/") end

    -- 从 tilemap collision 层创建墙壁实体
    local map_data = dofile("joy_beat/tilemap/map.lua")
    if map_data and map_data.layers then
        for _, layer in ipairs(map_data.layers) do
            if layer.name == "collision" and layer.objects then
                for _, obj in ipairs(layer.objects) do
                    if obj.shape == "rectangle" and obj.width > 0 and obj.height > 0 then
                        local mx = obj.x / PPM + obj.width / PPM / 2
                        local my = obj.y / PPM + obj.height / PPM / 2
                        create_wall(mx, my, obj.width / PPM, obj.height / PPM)
                    end
                end
                break
            end
        end
    end

    create_enemy(12, 5)
    console.log("walls created: " .. #entity_list.wall)
    console.log("map size: " .. map_data.width .. "x" .. map_data.height .. " tiles")
    local ok_srv, svr = pcall(net.create, "kcp", server_ip, server_port)
    if ok_srv and svr then server = svr; set_env("server", svr) end
    console.log("WASD=move SPACE=attack Q=quit")
end

function joy.hotfix()
    server = get_env("server")
end

function joy.update(dt)
    -- 摄像机跟随第一个玩家
    local lp = entity_list.player and entity_list.player[1]
    if lp then local lerp = math.min(dt * 5, 1)
        camera.x = camera.x + (lp.pos.x - camera.x) * lerp
        camera.y = camera.y + (lp.pos.y - camera.y) * lerp
    end

    -- 固定步长物理
    accumulator = accumulator + dt
    while accumulator >= FIXED_DT do
        fixed_update(FIXED_DT)
        frame_id = frame_id + 1
        accumulator = accumulator - FIXED_DT
    end

    -- 网络 + 帧同步（先处理网络消息再广播）
    process_network()
    net_timer = net_timer + dt
    while net_timer >= SERVER_TICK do
        save_world_snapshot()
        if server and net.connection_count(server) > 0 then
            net.broadcast(server, pack_world_state())
        end
        net_timer = net_timer - SERVER_TICK
    end
end

function joy.draw()
    joy.r2d.begin(ren, 30, 35, 45, 255)
    local ms = joy.sdl.get_ticks()
    if tilemap then joy.tilemap.render_all(tilemap, camera.x*PPM-480, camera.y*PPM-360, ms) end

    for _, ray in ipairs(attack_rays) do
        local sx, sy = to_screen(ray.x, ray.y)
        local a = math.floor(ray.lifetime/0.15*200)
        joy.r2d.set_color(ren, 255, 220, 80, a)
        joy.r2d.rectangle(ren, "fill", sx-ray.w/2*PPM, sy-ray.h/2*PPM, ray.w*PPM, ray.h*PPM)
    end

    local function draw_sprite(e)
        if not e.anim then
            -- fallback: colored rect
            local sx, sy = to_screen(e.pos.x, e.pos.y); local sw, sh = e.size.w*PPM, e.size.h*PPM
            local r, g = (e.type=="player") and 80 or 220, (e.type=="player") and 130 or 80
            if e.action=="attack" then r,g=255,200 end
            joy.r2d.set_color(ren, r, g, (e.type=="player") and 255 or 80, 255)
            joy.r2d.rectangle(ren, "fill", sx-sw/2, sy-sh/2, sw, sh)
        else
            local sx, sy = to_screen(e.pos.x, e.pos.y)
            -- janimator 内部已经根据 flip_h 处理镜像, 绘制时不需要再翻转
            joy.animator.draw(e.anim, sx - FW/2, sy - FH/2)
        end
        if e.max_hp and e.hp then
            local sx, sy = to_screen(e.pos.x, e.pos.y)
            local bw, ratio = 40, e.hp/e.max_hp
            joy.r2d.set_color(ren, 60,60,60,200); joy.r2d.rectangle(ren, "fill", sx-bw/2, sy-FH/2-10, bw, 4)
            joy.r2d.set_color(ren, 60,200,60,255); joy.r2d.rectangle(ren, "fill", sx-bw/2, sy-FH/2-10, bw*ratio, 4)
        end
    end

    for _, e in ipairs(entity_list.enemy) do draw_sprite(e) end
    for _, e in ipairs(entity_list.player) do draw_sprite(e) end

    -- 调试：绘制墙壁碰撞体（半透明蓝色边框）
    joy.r2d.set_color(ren, 50, 100, 255, 60)
    for _, w in ipairs(entity_list.wall) do
        local sx, sy = to_screen(w.pos.x, w.pos.y)
        joy.r2d.rectangle(ren, "line", sx-w.size.w/2*PPM, sy-w.size.h/2*PPM, w.size.w*PPM, w.size.h*PPM)
    end

    local hpp = entity_list.player and entity_list.player[1]
    -- 服务端 IP 信息
    local status = server and ("ws://" .. server_ip .. ":" .. server_port) or "server offline"
    joy.r2d.set_color(ren, 30, 30, 40, 200)
    joy.r2d.rectangle(ren, "fill", 10, 10, 340, 20)
    joy.r2d.set_color(ren, 150, 150, 180, 255)
    joy.r2d.print(ren, font, status, 15, 12, 150, 150, 180, 255)
    if hpp then
        joy.r2d.set_color(ren, 40, 40, 50, 200)
        joy.r2d.rectangle(ren, "fill", 10, 35, 320, 35)
        -- 用彩色矩形条指示 HP
        joy.r2d.set_color(ren, 60, 60, 60, 200)
        joy.r2d.rectangle(ren, "fill", 15, 40, 200, 10)
        joy.r2d.set_color(ren, 60, 200, 60, 255)
        joy.r2d.rectangle(ren, "fill", 15, 40, 200 * hpp.hp / hpp.max_hp, 10)
        -- 用圆点表示玩家数量
        for i = 1, #entity_list.player do
            joy.r2d.set_color(ren, 80, 130, 255, 255)
            joy.r2d.circle(ren, "fill", 15 + i * 25, 65, 6, 12)
        end
    end
    joy.r2d.end_frame(ren)
end

function joy.keypressed(key)
    if key=="q" or key=="escape" then joy.event.quit() end
end

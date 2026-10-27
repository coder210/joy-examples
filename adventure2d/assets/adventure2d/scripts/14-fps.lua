-- 3D FPS Demo（移植自 main8.cpp）
local PIXELS_PER_METER = 100

local r3d, win
local cube_mesh, sphere_mesh, floor_mesh, ramp_mesh
local checker_tex
local yaw, pitch = 0, 25
local player_pos = {x=0, y=0.6, z=0}
local vel_y = 0
local on_ground = false

-- 障碍物
local boxes = {}       -- {pos={}, size={}, collider}
local balls = {}       -- {pos={}, radius, collider}
local obbs = {}        -- {pos={}, size={}, angle, collider}

-- 子弹
local bullets = {}     -- {pos={}, dir={}, age}
local shoot_timer = 0

-- 输入状态
local keys = {w=false, s=false, a=false, d=false, space=false, f=false}

-- 碰撞体数组（用于碰撞检测时遍历）
local obstacles = {}

function joy.load()
    win = joy.window.create("3D FPS Demo", 1024, 720, 0)
    r3d = joy.render3d.create(win, "joy_editor/shaders/")
    set_env("fps_win", win)
    set_env("fps_r3d", r3d)

    -- 创建网格
    cube_mesh = joy.render3d.mesh_cube(r3d, 1, 1, 1)
    sphere_mesh = joy.render3d.mesh_sphere(r3d, 0.5, 16, 12)
    floor_mesh = joy.render3d.mesh_plane(r3d, 20, 20, 10)
    ramp_mesh = joy.render3d.mesh_ramp(r3d, 2, 3, 1.5)
    checker_tex = joy.render3d.gen_checkerboard(r3d, 16, 10)
    console.log("checkerboard tex:", tostring(checker_tex))

    -- 创建障碍物箱子
    local box_defs = {
        {x=3, z=3, sx=1, sy=1, sz=1},
        {x=-3, z=-2, sx=1, sy=0.5, sz=1},
        {x=5, z=-4, sx=0.8, sy=1.2, sz=0.8},
        {x=-4, z=4, sx=1, sy=1, sz=1},
        {x=-5, z=-4, sx=1.5, sy=0.8, sz=1.5},
        {x=4, z=-5, sx=0.5, sy=1.5, sz=0.5},
        {x=0, z=5, sx=1, sy=1, sz=1.5},
        {x=-1, z=-5, sx=1.2, sy=0.6, sz=1.2},
    }
    for _, d in ipairs(box_defs) do
        local c = joy.gjk3df.new_collider()
        local pos = {x=joy.fp.from_float(d.x), y=joy.fp.from_float(d.sy*0.5), z=joy.fp.from_float(d.z)}
        local half = {x=joy.fp.from_float(d.sx*0.5), y=joy.fp.from_float(d.sy*0.5), z=joy.fp.from_float(d.sz*0.5)}
        joy.gjk3df.init_box(c, pos, half)
        table.insert(boxes, {pos={x=d.x, y=d.sy*0.5, z=d.z}, size={x=d.sx, y=d.sy, z=d.sz}, collider=c})
        table.insert(obstacles, c)
    end

    -- 创建球体
    do
        local c = joy.gjk3df.new_collider()
        local pos = {x=joy.fp.from_float(2), y=joy.fp.from_float(0.5), z=joy.fp.from_float(2)}
        joy.gjk3df.init_sphere(c, pos, joy.fp.from_float(0.5))
        table.insert(balls, {pos={x=2, y=0.5, z=2}, radius=0.5, collider=c})
        table.insert(obstacles, c)
    end

    console.log("FPS demo loaded, boxes:", #boxes, "balls:", #balls)
end

function joy.draw()
    local dt = 1 / 60  -- 近似 60fps

    -- 处理输入
    local move_dir = {x=0, z=0}
    local yaw_rad = math.rad(yaw)
    local forward = {x=math.sin(yaw_rad), z=math.cos(yaw_rad)}
    local right = {x=math.cos(yaw_rad), z=-math.sin(yaw_rad)}

    if keys.w then move_dir.x = move_dir.x + forward.x; move_dir.z = move_dir.z + forward.z end
    if keys.s then move_dir.x = move_dir.x - forward.x; move_dir.z = move_dir.z - forward.z end
    if keys.a then move_dir.x = move_dir.x - right.x; move_dir.z = move_dir.z - right.z end
    if keys.d then move_dir.x = move_dir.x + right.x; move_dir.z = move_dir.z + right.z end

    -- 归一化移动方向
    local len = math.sqrt(move_dir.x * move_dir.x + move_dir.z * move_dir.z)
    if len > 0.001 then
        local speed = 5.0 * dt
        move_dir.x = move_dir.x / len * speed
        move_dir.z = move_dir.z / len * speed

        local new_x = player_pos.x + move_dir.x
        local new_z = player_pos.z + move_dir.z

        -- GJK 碰撞检测
        local player_half = {x=joy.fp.from_float(0.3), y=joy.fp.from_float(0.6), z=joy.fp.from_float(0.3)}
        local player_pos_fp = {x=joy.fp.from_float(new_x), y=joy.fp.from_float(player_pos.y), z=joy.fp.from_float(new_z)}

        local pc = joy.gjk3df.new_collider()
        joy.gjk3df.init_box(pc, player_pos_fp, player_half)
        local init_dir = {x=joy.fp.one(), y=joy.fp.zero(), z=joy.fp.zero()}

        local blocked = false
        for _, obs in ipairs(obstacles) do
            local r = joy.gjk3df.collide(pc, obs, init_dir)
            if r.hit then blocked = true; break end
        end

        if not blocked then
            -- 空气墙
            if new_x > 9.5 then new_x = 9.5 end
            if new_x < -9.5 then new_x = -9.5 end
            if new_z > 9.5 then new_z = 9.5 end
            if new_z < -9.5 then new_z = -9.5 end
            player_pos.x = new_x
            player_pos.z = new_z
        end
    end

    -- 跳跃/重力
    local ground_y = 0
    on_ground = (player_pos.y - 0.6) <= ground_y + 0.01
    if keys.space and on_ground then
        vel_y = 4.0
        on_ground = false
    end
    if not on_ground then
        vel_y = vel_y - 9.8 * dt
    end
    player_pos.y = player_pos.y + vel_y * dt
    if player_pos.y - 0.6 < ground_y then
        player_pos.y = ground_y + 0.6
        vel_y = 0
        on_ground = true
    end

    -- 射击
    shoot_timer = shoot_timer + dt
    if keys.f and shoot_timer >= 0.25 then
        shoot_timer = 0
        local dir = {x=math.sin(yaw_rad), y=0, z=math.cos(yaw_rad)}
        table.insert(bullets, {
            pos={x=player_pos.x + dir.x*0.6, y=player_pos.y+0.4, z=player_pos.z + dir.z*0.6},
            dir=dir, age=0
        })
    end

    -- 子弹更新
    for i = #bullets, 1, -1 do
        local b = bullets[i]
        b.age = b.age + dt
        b.pos.x = b.pos.x + b.dir.x * 15 * dt
        b.pos.y = b.pos.y + b.dir.y * 15 * dt
        b.pos.z = b.pos.z + b.dir.z * 15 * dt

        -- 超出边界或超时
        if math.abs(b.pos.x) > 10.5 or math.abs(b.pos.z) > 10.5 or b.age > 2 then
            table.remove(bullets, i)
        end
    end

    -- === 渲染 ===
    if not joy.render3d.begin(r3d) then return end

    -- 第三人称相机
    joy.render3d.camera_fps(r3d, player_pos, yaw, pitch, 45)

    -- 地板
    joy.render3d.draw_mesh(r3d, floor_mesh, {x=0,y=0,z=0}, {x=1,y=1,z=1}, {r=28,g=32,b=40,a=255})
    joy.render3d.draw_grid(r3d, 10, 1.0, 0.04)

    -- 斜坡（x=-6，main8.cpp 旧斜坡位置）
    joy.render3d.draw_mesh(r3d, ramp_mesh, {x=-6, y=0, z=0}, {x=1, y=1, z=1}, {r=180,g=120,b=80,a=255})

    -- 绘制障碍物箱子（浅蓝，main8.cpp 样式）
    for _, box in ipairs(boxes) do
        joy.render3d.draw_mesh(r3d, cube_mesh, box.pos, box.size, {r=60, g=68, b=80, a=255})
    end

    -- 绘制球体（绿色）
    for _, ball in ipairs(balls) do
        local pos = ball.pos
        local r = ball.radius
        joy.render3d.draw_mesh(r3d, sphere_mesh, pos, {x=r*2, y=r*2, z=r*2}, {r=40, g=220, b=40, a=255})
    end

    -- 绘制子弹（黄色小立方体）
    for _, b in ipairs(bullets) do
        joy.render3d.draw_mesh(r3d, cube_mesh, b.pos, {x=0.16, y=0.16, z=0.16}, {r=255, g=200, b=50, a=255})
    end

    -- 绘制玩家（半透明蓝色立方体）
    joy.render3d.draw_mesh(r3d, cube_mesh, player_pos, {x=0.6, y=1.2, z=0.6}, {r=100, g=150, b=255, a=180})

    joy.render3d.end_frame(r3d)
end

function joy.keypressed(key, scancode, rep, ctrl, shift)
    if key == "w" then keys.w = true end
    if key == "s" then keys.s = true end
    if key == "a" then keys.a = true end
    if key == "d" then keys.d = true end
    if key == "space" then keys.space = true end
    if key == "f" then keys.f = true end
    if key == "q" or key == "escape" then joy.event.quit() end
end

function joy.keyreleased(key, scancode)
    if key == "w" then keys.w = false end
    if key == "s" then keys.s = false end
    if key == "a" then keys.a = false end
    if key == "d" then keys.d = false end
    if key == "space" then keys.space = false end
    if key == "f" then keys.f = false end
end

function joy.mousemoved(x, y, dx, dy)
    yaw = yaw + dx * 0.05
end

function joy.hotfix()
    win = get_env("fps_win")
    r3d = get_env("fps_r3d")
end

function joy.quit()
    -- 先销毁 mesh，再销毁 r3d
    if ramp_mesh then joy.render3d.mesh_destroy(r3d, ramp_mesh); ramp_mesh = nil end
    if floor_mesh then joy.render3d.mesh_destroy(r3d, floor_mesh); floor_mesh = nil end
    if cube_mesh then joy.render3d.mesh_destroy(r3d, cube_mesh); cube_mesh = nil end
    if sphere_mesh then joy.render3d.mesh_destroy(r3d, sphere_mesh); sphere_mesh = nil end
    if r3d then joy.render3d.destroy(r3d); r3d = nil end
    console.log("FPS demo exit")
end

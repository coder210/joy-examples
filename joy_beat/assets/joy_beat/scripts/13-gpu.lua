-- 3D GPU 测试（raylib 风格 render3d API）
local r3d
local cube_mesh, sphere_mesh, floor_mesh
local yaw = 0

local win

function joy.load()
    win = joy.window.create("3D GPU Test", 1024, 720, 0)
    r3d = joy.render3d.create(win, "joy_editor/shaders/")
    set_env("gpu_win", win)
    set_env("gpu_r3d", r3d)
    console.log("render3d created:", tostring(r3d))

    -- 创建网格
    cube_mesh = joy.render3d.mesh_cube(r3d, 1, 1, 1)
    sphere_mesh = joy.render3d.mesh_sphere(r3d, 0.5, 16, 12)
    floor_mesh = joy.render3d.mesh_plane(r3d, 20, 20, 10)
    console.log("meshes created")
end

function joy.hotfix()
    win = get_env("gpu_win")
    r3d = get_env("gpu_r3d")
end

function joy.draw()
    yaw = yaw + 0.5  -- 每帧旋转 0.5°

    if not joy.render3d.begin(r3d) then return end

    -- 第三人称相机跟随原点
    joy.render3d.camera_fps(r3d, {x=0, y=1.5, z=0}, yaw, 25, 45)

    -- 地板
    joy.render3d.draw_mesh(r3d, floor_mesh, {x=0,y=0,z=0}, {x=1,y=1,z=1}, {r=28,g=32,b=40,a=255})
    -- 地板网格
    joy.render3d.draw_grid(r3d, 10, 1.0, 0.04)

    -- 绘制立方体
    joy.render3d.draw_mesh(r3d, cube_mesh,
        {x=0, y=0.5, z=0},
        {x=1.5, y=1.5, z=1.5},
        {r=60, g=120, b=220, a=255})

    -- 绘制球体
    joy.render3d.draw_mesh(r3d, sphere_mesh,
        {x=2.5, y=0.5, z=1.5},
        {x=1, y=1, z=1},
        {r=40, g=220, b=40, a=255})

    -- 另一个立方体
    joy.render3d.draw_mesh(r3d, cube_mesh,
        {x=-2.5, y=0.3, z=2},
        {x=0.6, y=0.6, z=0.6},
        {r=220, g=60, b=60, a=255})

    joy.render3d.end_frame(r3d)
end

function joy.keypressed(key, scancode, repeat_, ctrl, shift)
    if key == "q" or key == "escape" then joy.event.quit() end
end

function joy.quit()
    if floor_mesh then joy.render3d.mesh_destroy(r3d, floor_mesh) end
    if cube_mesh then joy.render3d.mesh_destroy(r3d, cube_mesh) end
    if sphere_mesh then joy.render3d.mesh_destroy(r3d, sphere_mesh) end
    if r3d then joy.render3d.destroy(r3d) end
    console.log("3D GPU demo exit")
end

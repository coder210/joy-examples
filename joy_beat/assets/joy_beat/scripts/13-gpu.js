// 3D GPU 测试（raylib 风格 render3d API）
var r3d;
var cube_mesh, sphere_mesh, floor_mesh;
var yaw = 0;
var win;

joy.load = function()
{
    win = get_env("gpu_win");
    r3d = get_env("gpu_r3d");
    if (!r3d) {
        win = window.create("3D GPU Test", 1024, 720, 0);
        r3d = render3d.create(win, "joy_editor/shaders/");
    console.log("render3d created");

    cube_mesh = render3d.mesh_cube(r3d, 1, 1, 1);
    sphere_mesh = render3d.mesh_sphere(r3d, 0.5, 16, 12);
    floor_mesh = render3d.mesh_plane(r3d, 20, 20, 10);
    console.log("meshes created");
    }
};

joy.hotfix = function() {
    if (win) set_env("gpu_win", win);
    if (r3d) set_env("gpu_r3d", r3d);
};

joy.draw = function()
{
    yaw = yaw + 0.5;

    if (!render3d.begin(r3d)) return;

    render3d.camera_fps(r3d, {x:0, y:1.5, z:0}, yaw, 25, 45);

    render3d.draw_mesh(r3d, floor_mesh, {x:0,y:0,z:0}, {x:1,y:1,z:1}, {r:28,g:32,b:40,a:255});
    render3d.draw_grid(r3d, 10, 1.0, 0.04);

    render3d.draw_mesh(r3d, cube_mesh,
        {x:0, y:0.5, z:0},
        {x:1.5, y:1.5, z:1.5},
        {r:60, g:120, b:220, a:255});

    render3d.draw_mesh(r3d, sphere_mesh,
        {x:2.5, y:0.5, z:1.5},
        {x:1, y:1, z:1},
        {r:40, g:220, b:40, a:255});

    render3d.draw_mesh(r3d, cube_mesh,
        {x:-2.5, y:0.3, z:2},
        {x:0.6, y:0.6, z:0.6},
        {r:220, g:60, b:60, a:255});

    render3d.end_frame(r3d);
};

joy.keypressed = function(key, scancode, repeat_, ctrl, shift)
{
    if (key == "q" || key == "escape") event.quit();
};

joy.quit = function()
{
    if (floor_mesh) render3d.mesh_destroy(r3d, floor_mesh);
    if (cube_mesh) render3d.mesh_destroy(r3d, cube_mesh);
    if (sphere_mesh) render3d.mesh_destroy(r3d, sphere_mesh);
    if (r3d) render3d.destroy(r3d);
    if (win) { window.destroy(win); win = null; }
    console.log("3D GPU demo exit");
};

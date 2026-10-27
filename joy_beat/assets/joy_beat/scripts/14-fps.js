// 3D FPS Demo (ported from main8.cpp)
var r3d, win;
var cube_mesh, sphere_mesh, floor_mesh;
var yaw = 0, pitch = 25;
var player_pos = {x:0, y:0.6, z:0};
var vel_y = 0;
var on_ground = false;

// obstacles
var boxes = [];
var balls = [];
var bullets = [];
var shoot_timer = 0;

// input
var keys = {w:false, s:false, a:false, d:false, space:false, f:false};
var obstacles = [];

joy.load = function()
{
    win = get_env("fps_win");
    r3d = get_env("fps_r3d");
    if (!r3d) {
        win = window.create("3D FPS Demo", 1024, 720, 0);
        r3d = render3d.create(win, "joy_editor/shaders/");

    cube_mesh   = render3d.mesh_cube(r3d, 1, 1, 1);
    sphere_mesh = render3d.mesh_sphere(r3d, 0.5, 16, 12);
    floor_mesh  = render3d.mesh_plane(r3d, 20, 20, 10);

    // boxes
    var box_defs = [
        {x:3, z:3, sx:1, sy:1, sz:1},
        {x:-3, z:-2, sx:1, sy:0.5, sz:1},
        {x:5, z:-4, sx:0.8, sy:1.2, sz:0.8},
        {x:-4, z:4, sx:1, sy:1, sz:1},
        {x:-5, z:-4, sx:1.5, sy:0.8, sz:1.5},
        {x:4, z:-5, sx:0.5, sy:1.5, sz:0.5},
        {x:0, z:5, sx:1, sy:1, sz:1.5},
        {x:-1, z:-5, sx:1.2, sy:0.6, sz:1.2},
    ];
    for (var i = 0; i < box_defs.length; i++) {
        var d = box_defs[i];
        var c = gjk3df.new_collider();
        var pos  = {x:fp.from_float(d.x), y:fp.from_float(d.sy*0.5), z:fp.from_float(d.z)};
        var half = {x:fp.from_float(d.sx*0.5), y:fp.from_float(d.sy*0.5), z:fp.from_float(d.sz*0.5)};
        gjk3df.init_box(c, pos, half);
        boxes.push({pos:{x:d.x, y:d.sy*0.5, z:d.z}, size:{x:d.sx, y:d.sy, z:d.sz}, collider:c});
        obstacles.push(c);
    }

    // sphere
    var c = gjk3df.new_collider();
    gjk3df.init_sphere(c, {x:fp.from_float(2), y:fp.from_float(0.5), z:fp.from_float(2)}, fp.from_float(0.5));
    balls.push({pos:{x:2, y:0.5, z:2}, radius:0.5, collider:c});
    obstacles.push(c);

    console.log("FPS demo loaded, boxes: " + boxes.length + " balls: " + balls.length);
    }
};

joy.hotfix = function() {
    if (win) set_env("fps_win", win);
    if (r3d) set_env("fps_r3d", r3d);
};

joy.draw = function()
{
    var dt = 1 / 60;

    // input → movement
    var move_dir = {x:0, z:0};
    var yaw_rad = yaw * 3.14159265 / 180;
    var fwd = {x:Math.sin(yaw_rad), z:Math.cos(yaw_rad)};
    var right = {x:Math.cos(yaw_rad), z:-Math.sin(yaw_rad)};

    if (keys.w) { move_dir.x += fwd.x; move_dir.z += fwd.z; }
    if (keys.s) { move_dir.x -= fwd.x; move_dir.z -= fwd.z; }
    if (keys.a) { move_dir.x -= right.x; move_dir.z -= right.z; }
    if (keys.d) { move_dir.x += right.x; move_dir.z += right.z; }

    var len = Math.sqrt(move_dir.x*move_dir.x + move_dir.z*move_dir.z);
    if (len > 0.001) {
        var speed = 5.0 * dt;
        move_dir.x = move_dir.x / len * speed;
        move_dir.z = move_dir.z / len * speed;

        var new_x = player_pos.x + move_dir.x;
        var new_z = player_pos.z + move_dir.z;

        // GJK collision
        var player_half = {x:fp.from_float(0.3), y:fp.from_float(0.6), z:fp.from_float(0.3)};
        var player_fp = {x:fp.from_float(new_x), y:fp.from_float(player_pos.y), z:fp.from_float(new_z)};
        var pc = gjk3df.new_collider();
        gjk3df.init_box(pc, player_fp, player_half);
        var init_dir = {x:fp.one(), y:fp.zero(), z:fp.zero()};

        var blocked = false;
        for (var j = 0; j < obstacles.length; j++) {
            var r = gjk3df.collide(pc, obstacles[j], init_dir);
            if (r.hit) { blocked = true; break; }
        }
        gjk3df.destroy(pc);

        if (!blocked) {
            if (new_x > 9.5) new_x = 9.5;
            if (new_x < -9.5) new_x = -9.5;
            if (new_z > 9.5) new_z = 9.5;
            if (new_z < -9.5) new_z = -9.5;
            player_pos.x = new_x;
            player_pos.z = new_z;
        }
    }

    // gravity / jump
    var ground_y = 0;
    on_ground = (player_pos.y - 0.6) <= ground_y + 0.01;
    if (keys.space && on_ground) { vel_y = 4.0; on_ground = false; }
    if (!on_ground) vel_y -= 9.8 * dt;
    player_pos.y += vel_y * dt;
    if (player_pos.y - 0.6 < ground_y) { player_pos.y = ground_y + 0.6; vel_y = 0; on_ground = true; }

    // shoot
    shoot_timer += dt;
    if (keys.f && shoot_timer >= 0.25) {
        shoot_timer = 0;
        var dir = {x:Math.sin(yaw_rad), y:0, z:Math.cos(yaw_rad)};
        bullets.push({pos:{x:player_pos.x+dir.x*0.6, y:player_pos.y+0.4, z:player_pos.z+dir.z*0.6}, dir:dir, age:0});
    }

    for (var i = bullets.length-1; i >= 0; i--) {
        var b = bullets[i];
        b.age += dt;
        b.pos.x += b.dir.x * 15 * dt;
        b.pos.y += b.dir.y * 15 * dt;
        b.pos.z += b.dir.z * 15 * dt;
        if (Math.abs(b.pos.x) > 10.5 || Math.abs(b.pos.z) > 10.5 || b.age > 2)
            bullets.splice(i, 1);
    }

    // === render ===
    if (!render3d.begin(r3d)) return;

    render3d.camera_fps(r3d, player_pos, yaw, pitch, 45);

    render3d.draw_mesh(r3d, floor_mesh, {x:0,y:0,z:0}, {x:1,y:1,z:1}, {r:28,g:32,b:40,a:255});
    render3d.draw_grid(r3d, 10, 1.0, 0.04);

    for (var i = 0; i < boxes.length; i++) {
        var box = boxes[i];
        render3d.draw_mesh(r3d, cube_mesh, box.pos, box.size, {r:60,g:68,b:80,a:255});
    }

    for (var i = 0; i < balls.length; i++) {
        var ball = balls[i];
        render3d.draw_mesh(r3d, sphere_mesh, ball.pos, {x:ball.radius*2,y:ball.radius*2,z:ball.radius*2}, {r:40,g:220,b:40,a:255});
    }

    for (var i = 0; i < bullets.length; i++) {
        var b = bullets[i];
        render3d.draw_mesh(r3d, cube_mesh, b.pos, {x:0.16,y:0.16,z:0.16}, {r:255,g:200,b:50,a:255});
    }

    render3d.draw_mesh(r3d, cube_mesh, player_pos, {x:0.6,y:1.2,z:0.6}, {r:100,g:150,b:255,a:180});

    render3d.end_frame(r3d);
};

joy.keypressed = function(key, scancode, rep, ctrl, shift)
{
    if (key == "w") keys.w = true;
    if (key == "s") keys.s = true;
    if (key == "a") keys.a = true;
    if (key == "d") keys.d = true;
    if (key == "space") keys.space = true;
    if (key == "f") keys.f = true;
    if (key == "q" || key == "escape") event.quit();
};

joy.keyreleased = function(key, scancode)
{
    if (key == "w") keys.w = false;
    if (key == "s") keys.s = false;
    if (key == "a") keys.a = false;
    if (key == "d") keys.d = false;
    if (key == "space") keys.space = false;
    if (key == "f") keys.f = false;
};

joy.mousemoved = function(x, y, dx, dy)
{
    yaw = yaw + dx * 0.05;
};

joy.quit = function()
{
    if (floor_mesh)  render3d.mesh_destroy(r3d, floor_mesh);
    if (cube_mesh)   render3d.mesh_destroy(r3d, cube_mesh);
    if (sphere_mesh) render3d.mesh_destroy(r3d, sphere_mesh);
    if (r3d)         render3d.destroy(r3d);
    if (win)         { window.destroy(win); win = null; }
    console.log("FPS demo exit");
};

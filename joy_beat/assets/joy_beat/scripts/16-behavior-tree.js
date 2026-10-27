// 行为树 demo（JS 版，C 绑定完整功能）
var win, ren, fnt;
var bt_root;
var elapsed = 0;

var ai = {
    hp: 100,
    target_x: 380,
    target_y: 300,
    my_x: 200,
    my_y: 300,
    speed: 150,
};

joy.load = function()
{
    win = get_env("bt_win");
    ren = get_env("bt_ren");
    fnt = get_env("bt_font");
    if (!fnt) {
        win = window.create("behavior-tree", 800, 600, 0);
        ren = r2d.create(win);
        fnt = font.create("joy_beat/fonts/simhei.ttf", 18);
        set_env("bt_win", win);
        set_env("bt_ren", ren);
        set_env("bt_font", fnt);
    }

    var bt = behavior;

    bt_root = bt.selector([
        bt.sequence([
            bt.condition(function() { return ai.hp < 30; }),
            bt.action(function(dt) {
                ai.my_x = ai.my_x + 150 * dt;
                ai.hp = ai.hp + 8 * dt;
                return "running";
            }),
        ]),
        bt.sequence([
            bt.action(function(dt) {
                elapsed = elapsed + dt;
                ai.my_x = ai.my_x + Math.sin(elapsed) * 80 * dt;
                ai.my_y = ai.my_y + Math.cos(elapsed * 0.7) * 80 * dt;
                var dx = ai.target_x - ai.my_x;
                var dy = ai.target_y - ai.my_y;
                if (Math.sqrt(dx * dx + dy * dy) < 200) return "success";
                return "running";
            }),
            bt.action(function(dt) {
                var dx = ai.target_x - ai.my_x;
                var dy = ai.target_y - ai.my_y;
                var dist = Math.sqrt(dx * dx + dy * dy);
                if (dist > 350) return "failure";
                ai.my_x = ai.my_x + (dx / dist) * ai.speed * dt;
                ai.my_y = ai.my_y + (dy / dist) * ai.speed * dt;
                if (dist < 40) return "success";
                return "running";
            }),
            bt.wait(0.3),
            bt.repeater(bt.action(function(dt) {
                var dx = ai.target_x - ai.my_x;
                var dy = ai.target_y - ai.my_y;
                if (Math.sqrt(dx * dx + dy * dy) > 80) return "failure";
                ai.hp = ai.hp - 10 * dt;
                if (ai.hp <= 0) { ai.hp = 100; console.log("target defeated!"); return "success"; }
                return "running";
            }), 4),
        ]),
    ]);
};

joy.hotfix = function() {
    if (win) set_env("bt_win", win);
    if (ren) set_env("bt_ren", ren);
    if (fnt) set_env("bt_font", fnt);
};

joy.update = function(dt) { if (bt_root) behavior.run(bt_root, dt); };

joy.draw = function()
{
    r2d.begin(ren, 30, 30, 40, 255);
    r2d.set_color(ren, 100, 200, 255, 255);
    r2d.circle(ren, "fill", ai.my_x, ai.my_y, 20, 32);
    r2d.set_color(ren, 255, 100, 100, 255);
    r2d.circle(ren, "fill", ai.target_x, ai.target_y, 10, 32);
    r2d.set_color(ren, 200, 200, 80, 60);
    r2d.circle(ren, "line", ai.my_x, ai.my_y, 200, 48);
    var dx = ai.target_x - ai.my_x, dy = ai.target_y - ai.my_y, dist = Math.sqrt(dx*dx + dy*dy);
    r2d.set_color(ren, 255, 255, 255, 255);
    r2d.print(ren, fnt, "hp: " + Math.floor(ai.hp), 10, 20, {r:255,g:255,b:255,a:255});
    r2d.print(ren, fnt, "dist: " + Math.floor(dist), 10, 45, {r:200,g:200,b:255,a:255});
    r2d.print(ren, fnt, "WASD=move  SPACE=attack  Q=quit", 10, 550, {r:180,g:180,b:180,a:255});
    r2d.end_frame(ren);
};

joy.keypressed = function(key)
{
    if (key === "escape" || key === "q") { event.quit(); return; }
    if (key === "space") { ai.hp -= 15; return; }
    if (key === "w") { ai.target_y -= 20; }
    if (key === "s") { ai.target_y += 20; }
    if (key === "a") { ai.target_x -= 20; }
    if (key === "d") { ai.target_x += 20; }
};

// 骑士动画状态机演示 (warrior_blue.png)
// WASD=移动  Space=攻击  Q=退出
var TEX = "joy_beat/textures/knights/troops/warrior/warrior_blue.png";
var FW = 192, FH = 192, FRAMES = 6;
var dir = 0;

var win, ren, anim;

joy.load = function()
{
    win = get_env("anim_win");
    ren = get_env("anim_ren");
    anim = get_env("anim_anim");
    if (!anim) {
        win = window.create("animator", 800, 600, 0);
        ren = r2d.create(win);
        anim = animator.create(ren);
    }
    set_env("anim_win", win);
    set_env("anim_ren", ren);
    set_env("anim_anim", anim);

    animator.set_float(anim, "speed", 0);
    animator.set_float(anim, "dir", 0);

    // Idle (行0, 循环)
    animator.add_state(anim, "idle");
    for (var col = 0; col < FRAMES; col++)
        animator.state_add_clip(anim, "idle", TEX, 0.15, col*FW, 0, FW, FH);
    animator.state_set_loop(anim, "idle", true);

    // Walk (行1, 循环)
    animator.add_state(anim, "walk");
    for (var col = 0; col < FRAMES; col++)
        animator.state_add_clip(anim, "walk", TEX, 0.12, col*FW, 1*FH, FW, FH);
    animator.state_set_loop(anim, "walk", true);

    // 4方向攻击 (非循环, exit_time令其播完才回idle)
    var ATK_ROWS = [4, 2, 2, 6];
    var ATK_NAMES = ["attack_d", "attack_l", "attack_r", "attack_u"];
    for (var d = 0; d < 4; d++) {
        animator.add_state(anim, ATK_NAMES[d]);
        for (var col = 0; col < FRAMES; col++)
            animator.state_add_clip(anim, ATK_NAMES[d], TEX, 0.08,
                col*FW, ATK_ROWS[d]*FH, FW, FH);
        animator.state_set_loop(anim, ATK_NAMES[d], false);
        var t = animator.add_transition(anim, ATK_NAMES[d], "idle");
        animator.transition_set_exit_time(anim, t, 0.08 * FRAMES);  // 0.48秒后回idle
    }

    animator.set_default_state(anim, "idle");

    // idle ↔ walk (speed 条件)
    var t = animator.add_transition(anim, "idle", "walk");
    animator.transition_add_cond(anim, t, "speed", 0, 0.1);
    t = animator.add_transition(anim, "walk", "idle");
    animator.transition_add_cond(anim, t, "speed", 1, 0.1);

    // Any State → 攻击 (trigger + dir)
    for (var d = 0; d < 4; d++) {
        t = animator.add_any_transition(anim, ATK_NAMES[d]);
        animator.transition_add_cond(anim, t, "atk_trig", 6, 0);
        animator.transition_add_cond(anim, t, "dir", 2, d);
    }

    console.log("WASD移动 Space攻击 Q退出");
};

joy.hotfix = function() {
    win = get_env("anim_win");
    ren = get_env("anim_ren");
    anim = get_env("anim_anim");
};

joy.update = function(dt)
{
    var cur = animator.current_state(anim);
    // 攻击中不处理移动输入,攻击结束自动回idle(exit_time到期后)
    animator.update(anim, dt);
};

joy.draw = function()
{
    r2d.begin(ren, 40, 40, 50, 255);
    animator.draw(anim, 280, 180);
    r2d.end_frame(ren);
};

joy.keypressed = function(key)
{
    if (key === "escape" || key === "q") {
        event.quit();
    } else if (key === "space") {
        var cur = animator.current_state(anim);
        if (cur.indexOf("attack") !== 0)
            animator.set_trigger(anim, "atk_trig");
    } else if (key === "w") { dir = 3; animator.set_float(anim, "speed", 1); animator.set_float(anim, "dir", 3); animator.set_flip_h(anim, false);
    } else if (key === "s") { dir = 0; animator.set_float(anim, "speed", 1); animator.set_float(anim, "dir", 0); animator.set_flip_h(anim, false);
    } else if (key === "a") { dir = 1; animator.set_float(anim, "speed", 1); animator.set_float(anim, "dir", 1); animator.set_flip_h(anim, true);
    } else if (key === "d") { dir = 2; animator.set_float(anim, "speed", 1); animator.set_float(anim, "dir", 2); animator.set_flip_h(anim, false); }
};

joy.keyreleased = function(key)
{
    if (key === "w" || key === "s" || key === "a" || key === "d") {
        animator.set_float(anim, "speed", 0);
    }
};

joy.quit = function()
{
    if (anim) { animator.destroy(anim); anim = null; }
    if (ren) { r2d.destroy(ren); ren = null; }
    if (win) { window.destroy(win); win = null; }
};

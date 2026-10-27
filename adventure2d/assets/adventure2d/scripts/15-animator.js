// Animator demo with hotfix persistence
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
    if (anim) {
        animator.add_state(anim, "test");
        animator.state_add_color(anim, "test", 0.2, 128, 128, 255, 0, 0, 255);
        animator.state_add_color(anim, "test", 0.2, 128, 128, 0, 255, 0, 255);
        animator.state_add_color(anim, "test", 0.2, 128, 128, 0, 0, 255, 255);
        animator.state_add_color(anim, "test", 0.2, 128, 128, 255, 255, 0, 255);
        animator.state_set_loop(anim, "test", true);
        console.log("loaded, state=" + animator.current_state(anim));
    }
};

joy.update = function(dt)
{
    if (anim) animator.update(anim, dt);
};

joy.draw = function()
{
    if (anim) {
        r2d.begin(ren, 40, 40, 50, 255);
        animator.draw(anim, 280, 180);
        r2d.end_frame(ren);
    }
};

joy.hotfix = function()
{
    if (win) set_env("anim_win", win);
    if (ren) set_env("anim_ren", ren);
    if (anim) set_env("anim_anim", anim);
};

joy.quit = function()
{
    if (anim) animator.destroy(anim);
    if (ren) { r2d.destroy(ren); ren = null; }
    if (win) { window.destroy(win); win = null; }
};

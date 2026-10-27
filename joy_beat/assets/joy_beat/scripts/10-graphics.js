// 2D Graphics drawing demo
var ren;

joy.load = function()
{
    ren = get_env("gfx_ren");
    if (!ren) {
        var win = window.create("graphics", 800, 600, 0);
        ren = r2d.create(win);
        set_env("gfx_ren", ren);
    }
};

joy.draw = function()
{
    r2d.begin(ren, 0, 0, 0, 255);
    r2d.set_color(ren, 255, 0, 0, 255);
    r2d.rectangle(ren, "fill", 100, 100, 200, 150);
    r2d.set_color(ren, 0, 255, 0, 255);
    r2d.circle(ren, "fill", 400, 300, 80, 32);
    r2d.end_frame(ren);
};

joy.keypressed = function(key, scancode, repeat_, ctrl, shift)
{
    if (key === "escape") event.quit();
};

joy.hotfix = function() {
    if (ren) set_env("gfx_ren", ren);
};

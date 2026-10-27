// 2D Graphics drawing demo
// Note: JS polygon not yet implemented, using lines to approximate
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

joy.draw = function() {};

joy.hotfix = function() {
    if (ren) set_env("gfx_ren", ren);
};

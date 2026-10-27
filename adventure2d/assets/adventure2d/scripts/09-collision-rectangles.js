// 2D Rectangle vs Rectangle collision detection demo
var PIXELS_PER_METER = 100;
var rect1, rect2;
var ren;

joy.load = function()
{
    ren = get_env("c09_ren");
    if (!ren) {
        var win = window.create("rectangles", 720, 480, 0);
        ren = r2d.create(win);
        set_env("c09_ren", ren);
    }
};

joy.draw = function() {};

joy.hotfix = function() {
    if (ren) set_env("c09_ren", ren);
};

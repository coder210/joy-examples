// 2D Ray vs Rectangle collision detection demo
var PIXELS_PER_METER = 100;
var rect = { x: fp.from_float(2), y: fp.from_float(2),
             w: fp.from_float(2), h: fp.from_float(2) };
var ray = { origin: { x: fp.from_float(0), y: fp.from_float(0) },
            dir: { x: fp.from_float(1), y: fp.from_float(0.77) } };
var ren;

joy.load = function()
{
    ren = get_env("c08_ren");
    if (!ren) {
        var win = window.create("ray-rect", 720, 480, 0);
        ren = r2d.create(win);
        set_env("c08_ren", ren);
    }
};

joy.draw = function() {};

joy.hotfix = function() {
    if (ren) set_env("c08_ren", ren);
};

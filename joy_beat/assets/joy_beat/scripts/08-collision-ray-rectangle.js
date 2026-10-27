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

joy.draw = function()
{
    r2d.begin(ren, 0, 0, 0, 255);
    var result = collision2df.get_ray_rectangle(ray.origin, ray.dir, rect);
    if (result.hit) {
        r2d.set_color(ren, 255, 0, 0, 255);
        var px = fp.to_float(result.point.x) * PIXELS_PER_METER;
        var py = fp.to_float(result.point.y) * PIXELS_PER_METER;
        r2d.circle(ren, "fill", px, py, 5, 16);
    } else {
        r2d.set_color(ren, 0, 255, 0, 255);
    }
    var rx = fp.to_float(rect.x) * PIXELS_PER_METER;
    var ry = fp.to_float(rect.y) * PIXELS_PER_METER;
    var rw = fp.to_float(rect.w) * PIXELS_PER_METER;
    var rh = fp.to_float(rect.h) * PIXELS_PER_METER;
    r2d.rectangle(ren, "line", rx, ry, rw, rh);
    r2d.set_color(ren, 255, 255, 255, 255);
    r2d.line(ren, 0, 0,
        fp.to_float(ray.origin.x + ray.dir.x * 5) * PIXELS_PER_METER,
        fp.to_float(ray.origin.y + ray.dir.y * 5) * PIXELS_PER_METER);
    r2d.end_frame(ren);
};

joy.keypressed = function(key, scancode, repeat_, ctrl, shift, alt)
{
    if (key === "escape") event.quit();
};

joy.hotfix = function() {
    if (ren) set_env("c08_ren", ren);
};

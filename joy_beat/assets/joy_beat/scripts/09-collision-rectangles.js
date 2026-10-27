// 2D Rectangle vs Rectangle collision detection demo
var PIXELS_PER_METER = 100;
var rect1, rect2, angle1, angle2;
var ren;

joy.load = function()
{
    ren = get_env("c09_ren");
    if (!ren) {
        var win = window.create("rectangles", 720, 480, 0);
        ren = r2d.create(win);
        set_env("c09_ren", ren);
    }
    rect1 = { x: 2, y: 2, w: 2, h: 1 };
    rect2 = { x: 2.5, y: 2.5, w: 2, h: 1 };
    angle1 = 0;
    angle2 = 30;
};

joy.draw = function()
{
    r2d.set_color(ren, 0, 0, 0, 255);
    r2d.begin(ren);
    var hit = collision2df.rectangles(
        { x: fp.from_float(rect1.x), y: fp.from_float(rect1.y),
          w: fp.from_float(rect1.w), h: fp.from_float(rect1.h) },
        { x: fp.from_float(rect2.x), y: fp.from_float(rect2.y),
          w: fp.from_float(rect2.w), h: fp.from_float(rect2.h) });
    if (hit) r2d.set_color(ren, 255, 0, 0, 255);
    else r2d.set_color(ren, 0, 255, 0, 255);

    var rx1 = rect1.x * PIXELS_PER_METER;
    var ry1 = rect1.y * PIXELS_PER_METER;
    var rw1 = rect1.w * PIXELS_PER_METER;
    var rh1 = rect1.h * PIXELS_PER_METER;
    r2d.rectangle(ren, "line", rx1, ry1, rw1, rh1);

    var rx2 = rect2.x * PIXELS_PER_METER;
    var ry2 = rect2.y * PIXELS_PER_METER;
    var rw2 = rect2.w * PIXELS_PER_METER;
    var rh2 = rect2.h * PIXELS_PER_METER;
    r2d.rectangle(ren, "line", rx2, ry2, rw2, rh2);
    r2d.end_frame(ren);
};

joy.keypressed = function(key, scancode, repeat_, ctrl, shift, alt)
{
    if (key === "escape") event.quit();
};

joy.hotfix = function() {
    if (ren) set_env("c09_ren", ren);
};

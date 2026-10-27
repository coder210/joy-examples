// Circle vs Circle collision detection demo
var PIXELS_PER_METER = 100;
var circle1, circle2;
var ren;
var win;

joy.load = function () {
    win = window.create("circles", 720, 480, 0);
    ren = r2d.create(win);
    circle1 = { x: fp.from_float(3), y: fp.from_float(2.5), r: fp.from_float(0.8) };
    circle2 = { x: fp.from_float(5), y: fp.from_float(2.5), r: fp.from_float(0.8) };
    set_env("c05_circle1", circle1);
    set_env("c05_circle2", circle2);
    get_env("c05_win", win);
    set_env("c05_ren", ren);
};

joy.mousemoved = function (x, y, dx, dy) {
    circle2.x = fp.from_float(x / PIXELS_PER_METER);
    circle2.y = fp.from_float(y / PIXELS_PER_METER);
};

joy.keypressed = function (key, scancode, repeat_, ctrl, shift, alt) {
    if (key == "escape") { event.quit() }
}

joy.draw = function () {
    r2d.begin(ren, 0, 0, 0, 255);

    var r = collision2df.circlesx(circle1, circle2);
    if (r.ok) {
        r2d.set_color(ren, 255, 0, 0, 255);
        var sp_x = fp.to_float(r.contact.sp.x) * PIXELS_PER_METER;
        var sp_y = fp.to_float(r.contact.sp.y) * PIXELS_PER_METER;
        var ep_x = fp.to_float(r.contact.ep.x) * PIXELS_PER_METER;
        var ep_y = fp.to_float(r.contact.ep.y) * PIXELS_PER_METER;
        var depth = fp.to_float(r.contact.depth);
        var nx = sp_x + depth * fp.to_float(r.contact.normal.x) * PIXELS_PER_METER;
        var ny = sp_y + depth * fp.to_float(r.contact.normal.y) * PIXELS_PER_METER;
        r2d.line(ren, sp_x, sp_y, ep_x, ep_y);
    } else {
        r2d.set_color(ren, 255, 255, 0, 255);
    }

    var x = fp.to_float(circle1.x) * PIXELS_PER_METER;
    var y = fp.to_float(circle1.y) * PIXELS_PER_METER;
    var rr = fp.to_float(circle1.r) * PIXELS_PER_METER;
    r2d.circle(ren, "line", x, y, rr, 50);

    x = fp.to_float(circle2.x) * PIXELS_PER_METER;
    y = fp.to_float(circle2.y) * PIXELS_PER_METER;
    rr = fp.to_float(circle2.r) * PIXELS_PER_METER;
    r2d.circle(ren, "line", x, y, rr, 50);

    r2d.end_frame(ren);
};

joy.hotfix = function () {
    win = get_env("c05_win");
    ren = get_env("c05_ren");
    circle1 = get_env("c05_circle1");
    circle2 = get_env("c05_circle2");
};

joy.quit = function () {
    if (ren) { r2d.destroy(ren); ren = null; }
    if (win) { window.destroy(win); win = null; }
};

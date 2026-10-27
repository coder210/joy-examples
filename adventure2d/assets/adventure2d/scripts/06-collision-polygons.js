// Polygon vs Polygon collision detection demo
var PIXELS_PER_METER = 100;
var poly_a, poly_b, poly_c;
var ren;
var win;

joy.load = function()
{
    win = get_env("c06_win");
    ren = get_env("c06_ren");
    if (!ren) {
        win = window.create("polygons", 720, 480, 0);
        ren = r2d.create(win);
    }

    poly_a = [
        {x: fp.from_float(1.0), y: fp.from_float(1.0)},
        {x: fp.from_float(2.0), y: fp.from_float(1.0)},
        {x: fp.from_float(2.0), y: fp.from_float(2.0)},
        {x: fp.from_float(1.0), y: fp.from_float(2.0)}
    ];
    poly_b = [
        {x: fp.from_float(1.5), y: fp.from_float(1.5)},
        {x: fp.from_float(2.5), y: fp.from_float(1.5)},
        {x: fp.from_float(2.5), y: fp.from_float(2.5)},
        {x: fp.from_float(1.5), y: fp.from_float(2.5)}
    ];
    poly_c = [
        {x: fp.from_float(4.5), y: fp.from_float(1.0)},
        {x: fp.from_float(5.5), y: fp.from_float(1.0)},
        {x: fp.from_float(5.0), y: fp.from_float(2.5)}
    ];
};

joy.hotfix = function() {
    if (win) set_env("c06_win", win);
    if (ren) set_env("c06_ren", ren);
};

joy.draw = function()
{
    r2d.begin(ren, 0, 0, 0, 255);

    // 碰撞对（poly_a vs poly_b，红色）
    var hit = collision2df.polygons(poly_a, poly_b);
    if (hit.ok)
        r2d.set_color(ren, 200, 50, 50, 255);
    else
        r2d.set_color(ren, 50, 200, 50, 255);

    // draw poly_a
    r2d.polygon(ren, "line",
        fp.to_float(poly_a[0].x) * PIXELS_PER_METER,
        fp.to_float(poly_a[0].y) * PIXELS_PER_METER,
        fp.to_float(poly_a[1].x) * PIXELS_PER_METER,
        fp.to_float(poly_a[1].y) * PIXELS_PER_METER,
        fp.to_float(poly_a[2].x) * PIXELS_PER_METER,
        fp.to_float(poly_a[2].y) * PIXELS_PER_METER,
        fp.to_float(poly_a[3].x) * PIXELS_PER_METER,
        fp.to_float(poly_a[3].y) * PIXELS_PER_METER);

    // draw poly_b
    r2d.polygon(ren, "line",
        fp.to_float(poly_b[0].x) * PIXELS_PER_METER,
        fp.to_float(poly_b[0].y) * PIXELS_PER_METER,
        fp.to_float(poly_b[1].x) * PIXELS_PER_METER,
        fp.to_float(poly_b[1].y) * PIXELS_PER_METER,
        fp.to_float(poly_b[2].x) * PIXELS_PER_METER,
        fp.to_float(poly_b[2].y) * PIXELS_PER_METER,
        fp.to_float(poly_b[3].x) * PIXELS_PER_METER,
        fp.to_float(poly_b[3].y) * PIXELS_PER_METER);

    // 无碰撞对（poly_c vs poly_a，绿色）
    var nohit = collision2df.polygons(poly_a, poly_c);
    if (nohit.ok)
        r2d.set_color(ren, 200, 50, 50, 255);
    else
        r2d.set_color(ren, 50, 200, 50, 255);

    r2d.polygon(ren, "line",
        fp.to_float(poly_c[0].x) * PIXELS_PER_METER,
        fp.to_float(poly_c[0].y) * PIXELS_PER_METER,
        fp.to_float(poly_c[1].x) * PIXELS_PER_METER,
        fp.to_float(poly_c[1].y) * PIXELS_PER_METER,
        fp.to_float(poly_c[2].x) * PIXELS_PER_METER,
        fp.to_float(poly_c[2].y) * PIXELS_PER_METER);

    r2d.end_frame(ren);
};

joy.keypressed = function(key, scancode, repeat_, ctrl, shift, alt) {
    if (key == "escape") event.quit();
};

joy.mousemoved = function(x, y, dx, dy) {
    var sx = dx / PIXELS_PER_METER;
    var sy = dy / PIXELS_PER_METER;
    for (var i = 0; i < poly_b.length; i++) {
        poly_b[i].x = poly_b[i].x + fp.from_float(sx);
        poly_b[i].y = poly_b[i].y + fp.from_float(sy);
    }
};

joy.quit = function() {
    if (ren) { r2d.destroy(ren); ren = null; }
    if (win) { window.destroy(win); win = null; }
};

-- Polygon vs Polygon collision detection demo
local PIXELS_PER_METER = 100
local poly_a, poly_b, poly_c

local ren
local win

function joy.load()
    win = get_env("c06_win")
    ren = get_env("c06_ren")
    if not ren then
        win = joy.window.create("polygons", 720, 480, 0)
        ren = joy.r2d.create(win)
    end

    poly_a = {
        {x = joy.fp.from_float(1.0), y = joy.fp.from_float(1.0)},
        {x = joy.fp.from_float(2.0), y = joy.fp.from_float(1.0)},
        {x = joy.fp.from_float(2.0), y = joy.fp.from_float(2.0)},
        {x = joy.fp.from_float(1.0), y = joy.fp.from_float(2.0)}
    }
    poly_b = {
        {x = joy.fp.from_float(1.5), y = joy.fp.from_float(1.5)},
        {x = joy.fp.from_float(2.5), y = joy.fp.from_float(1.5)},
        {x = joy.fp.from_float(2.5), y = joy.fp.from_float(2.5)},
        {x = joy.fp.from_float(1.5), y = joy.fp.from_float(2.5)}
    }
    poly_c = {
        {x = joy.fp.from_float(4.5), y = joy.fp.from_float(1.0)},
        {x = joy.fp.from_float(5.5), y = joy.fp.from_float(1.0)},
        {x = joy.fp.from_float(5.0), y = joy.fp.from_float(2.5)}
    }
end

function joy.hotfix()
    if win then set_env("c06_win", win) end
    if ren then set_env("c06_ren", ren) end
end

function joy.draw()
    joy.r2d.begin(ren, 0, 0, 0, 255)

    -- 碰撞对（poly_a vs poly_b，红色）
    local hit = joy.collision2df.polygons(poly_a, poly_b)
    if hit.ok then
        joy.r2d.set_color(ren, 200, 50, 50, 255)
    else
        joy.r2d.set_color(ren, 50, 200, 50, 255)
    end

    -- draw poly_a
    local pts = {}
    for _, v in ipairs(poly_a) do
        table.insert(pts, joy.fp.to_float(v.x) * PIXELS_PER_METER)
        table.insert(pts, joy.fp.to_float(v.y) * PIXELS_PER_METER)
    end
    joy.r2d.polygon(ren, "line", table.unpack(pts))

    -- draw poly_b
    pts = {}
    for _, v in ipairs(poly_b) do
        table.insert(pts, joy.fp.to_float(v.x) * PIXELS_PER_METER)
        table.insert(pts, joy.fp.to_float(v.y) * PIXELS_PER_METER)
    end
    joy.r2d.polygon(ren, "line", table.unpack(pts))

    -- 无碰撞对（poly_c vs poly_a，绿色）
    local nohit = joy.collision2df.polygons(poly_a, poly_c)
    if nohit.ok then
        joy.r2d.set_color(ren, 200, 50, 50, 255)
    else
        joy.r2d.set_color(ren, 50, 200, 50, 255)
    end
    pts = {}
    for _, v in ipairs(poly_c) do
        table.insert(pts, joy.fp.to_float(v.x) * PIXELS_PER_METER)
        table.insert(pts, joy.fp.to_float(v.y) * PIXELS_PER_METER)
    end
    joy.r2d.polygon(ren, "line", table.unpack(pts))

    joy.r2d.end_frame(ren)
end

function joy.keypressed(key, scancode, repeat_, ctrl, shift, alt)
    if key == "escape" then joy.event.quit() end
end

function joy.mousemoved(x, y, dx, dy)
    local sx = dx / PIXELS_PER_METER
    local sy = dy / PIXELS_PER_METER
    for _, v in ipairs(poly_b) do
        v.x = v.x + joy.fp.from_float(sx)
        v.y = v.y + joy.fp.from_float(sy)
    end
end

function joy.quit()
end

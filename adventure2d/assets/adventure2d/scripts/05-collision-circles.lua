-- Circle vs Circle collision detection demo
local PIXELS_PER_METER = 100
local circle1, circle2
local ren
local win

function joy.load()
    win = joy.window.create("circles", 720, 480, 0)
    ren = joy.r2d.create(win)
    set_env("c05_win", win)
    set_env("c05_ren", ren)

    circle1 = {
        x = joy.fp.from_float(2.2),
        y = joy.fp.from_float(2.2),
        r = joy.fp.from_float(0.64)
    }
    circle2 = {
        x = joy.fp.from_float(2.35),
        y = joy.fp.from_float(2.35),
        r = joy.fp.from_float(0.53)
    }

    set_env("c05_circle1", circle1)
    set_env("c05_circle2", circle2)
end

function joy.hotfix()
    win = get_env("c05_win")
    ren = get_env("c05_ren")
    circle1 = get_env("c05_circle1")
    circle2 = get_env("c05_circle2")
end

function joy.update(dt)
end

function joy.mousemoved(x, y, dx, dy)
    circle2.x = joy.fp.from_float(x / PIXELS_PER_METER)
    circle2.y = joy.fp.from_float(y / PIXELS_PER_METER)
end

function joy.draw()
    joy.r2d.begin(ren, 0, 0, 0, 255)

    local r = joy.collision2df.circlesx(circle1, circle2)
    if r.ok then
        joy.r2d.set_color(ren, 255, 0, 0, 255)
        local sp_x = joy.fp.to_float(r.contact.sp.x) * PIXELS_PER_METER
        local sp_y = joy.fp.to_float(r.contact.sp.y) * PIXELS_PER_METER
        local ep_x = joy.fp.to_float(r.contact.ep.x) * PIXELS_PER_METER
        local ep_y = joy.fp.to_float(r.contact.ep.y) * PIXELS_PER_METER
        local depth = joy.fp.to_float(r.contact.depth)
        local nx = sp_x + depth * joy.fp.to_float(r.contact.normal.x) * PIXELS_PER_METER
        local ny = sp_y + depth * joy.fp.to_float(r.contact.normal.y) * PIXELS_PER_METER
        joy.r2d.line(ren, sp_x, sp_y, ep_x, ep_y)
    else
        joy.r2d.set_color(ren, 255, 255, 0, 255)
    end

    local x = joy.fp.to_float(circle1.x) * PIXELS_PER_METER
    local y = joy.fp.to_float(circle1.y) * PIXELS_PER_METER
    local r = joy.fp.to_float(circle1.r) * PIXELS_PER_METER
    joy.r2d.circle(ren, "line", x, y, r, 50)

    x = joy.fp.to_float(circle2.x) * PIXELS_PER_METER
    y = joy.fp.to_float(circle2.y) * PIXELS_PER_METER
    r = joy.fp.to_float(circle2.r) * PIXELS_PER_METER
    joy.r2d.circle(ren, "line", x, y, r, 50)

    joy.r2d.end_frame(ren)
end

function joy.keypressed(key, scancode, repeat_, ctrl, shift, alt)
    if key == "escape" then joy.event.quit() end
end

function joy.quit()
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

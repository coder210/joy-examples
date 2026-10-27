-- 2D Ray vs Rectangle collision detection demo
local PIXELS_PER_METER = 100
local rect = { x = joy.fp.from_float(2), y = joy.fp.from_float(2),
               w = joy.fp.from_float(2), h = joy.fp.from_float(2) }
local ray = { origin = { x = joy.fp.from_float(0), y = joy.fp.from_float(0) },
              dir = { x = joy.fp.from_float(1), y = joy.fp.from_float(0.77) } }
local ren
local win

function joy.load()
    win = joy.window.create("ray-rect", 720, 480, 0)
    ren = joy.r2d.create(win)
    set_env("c08_win", win)
    set_env("c08_ren", ren)
end

function joy.hotfix()
    win = get_env("c08_win")
    ren = get_env("c08_ren")
end

function joy.draw()
    joy.r2d.begin(ren, 0, 0, 0, 255)
    local result = joy.collision2df.get_ray_rectangle(ray.origin, ray.dir, rect)
    if result.hit then
        joy.r2d.set_color(ren, 255, 0, 0, 255)
        local px = joy.fp.to_float(result.point.x) * PIXELS_PER_METER
        local py = joy.fp.to_float(result.point.y) * PIXELS_PER_METER
        joy.r2d.circle(ren, "fill", px, py, 5, 16)
    else
        joy.r2d.set_color(ren, 0, 255, 0, 255)
    end
    local rx = joy.fp.to_float(rect.x) * PIXELS_PER_METER
    local ry = joy.fp.to_float(rect.y) * PIXELS_PER_METER
    local rw = joy.fp.to_float(rect.w) * PIXELS_PER_METER
    local rh = joy.fp.to_float(rect.h) * PIXELS_PER_METER
    joy.r2d.rectangle(ren, "line", rx, ry, rw, rh)
    joy.r2d.set_color(ren, 255, 255, 255, 255)
    joy.r2d.line(ren, 0, 0,
        joy.fp.to_float(ray.origin.x + ray.dir.x * 5) * PIXELS_PER_METER,
        joy.fp.to_float(ray.origin.y + ray.dir.y * 5) * PIXELS_PER_METER)
    joy.r2d.end_frame(ren)
end

function joy.keypressed(key, scancode, repeat_, ctrl, shift, alt)
    if key == "escape" then joy.event.quit() end
end

function joy.quit()
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

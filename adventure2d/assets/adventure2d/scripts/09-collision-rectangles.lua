-- 2D Rectangle vs Rectangle collision detection demo
local PIXELS_PER_METER = 100
local rect1, rect2, angle1, angle2
local ren
local win

function joy.load()
    win = joy.window.create("rectangles", 720, 480, 0)
    ren = joy.r2d.create(win)
    set_env("c09_win", win)
    set_env("c09_ren", ren)
    rect1 = { x = 2, y = 2, w = 2, h = 1 }
    rect2 = { x = 2.5, y = 2.5, w = 2, h = 1 }
    angle1 = 0
    angle2 = 30
end

function joy.hotfix()
    win = get_env("c09_win")
    ren = get_env("c09_ren")
end

function joy.draw()
    joy.r2d.set_color(ren, 0, 0, 0, 255)
    joy.r2d.begin(ren)
    local hit = joy.collision2df.rectangles(
        { x = joy.fp.from_float(rect1.x), y = joy.fp.from_float(rect1.y),
          w = joy.fp.from_float(rect1.w), h = joy.fp.from_float(rect1.h) },
        { x = joy.fp.from_float(rect2.x), y = joy.fp.from_float(rect2.y),
          w = joy.fp.from_float(rect2.w), h = joy.fp.from_float(rect2.h) })
    if hit then joy.r2d.set_color(ren, 255, 0, 0, 255)
    else joy.r2d.set_color(ren, 0, 255, 0, 255) end
    local rx1 = rect1.x * PIXELS_PER_METER
    local ry1 = rect1.y * PIXELS_PER_METER
    local rw1 = rect1.w * PIXELS_PER_METER
    local rh1 = rect1.h * PIXELS_PER_METER
    joy.r2d.rectangle(ren, "line", rx1, ry1, rw1, rh1)
    local rx2 = rect2.x * PIXELS_PER_METER
    local ry2 = rect2.y * PIXELS_PER_METER
    local rw2 = rect2.w * PIXELS_PER_METER
    local rh2 = rect2.h * PIXELS_PER_METER
    joy.r2d.rectangle(ren, "line", rx2, ry2, rw2, rh2)
    joy.r2d.end_frame(ren)
end

function joy.keypressed(key, scancode, repeat_, ctrl, shift, alt)
    if key == "escape" then joy.event.quit() end
end

function joy.quit()
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

-- 2D Graphics drawing demo
local camera
local ren
local win

function joy.load()
    win = joy.window.create("graphics", 800, 600, 0)
    ren = joy.r2d.create(win)
    set_env("gfx_win", win)
    set_env("gfx_ren", ren)
end

function joy.hotfix()
    win = get_env("gfx_win")
    ren = get_env("gfx_ren")
end

function joy.draw()
    joy.r2d.begin(ren, 0, 0, 0, 255)
    joy.r2d.set_color(ren, 255, 0, 0, 255)
    joy.r2d.rectangle(ren, "fill", 100, 100, 200, 150)
    joy.r2d.set_color(ren, 0, 255, 0, 255)
    joy.r2d.circle(ren, "fill", 400, 300, 80, 32)
    joy.r2d.end_frame(ren)
end

function joy.keypressed(key, scancode, repeat_, ctrl, shift)
    if key == "escape" then joy.event.quit() end
end

function joy.quit()
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

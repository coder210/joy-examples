-- Audio playback demo with buttons
local device_id, bgm, shot
local ren, win

-- 按钮定义 {x, y, w, h, label, r, g, b, action}
local buttons = {}

function joy.load()
    win = get_env("audio_win")
    ren = get_env("audio_ren")
    device_id = get_env("audio_dev")
    if not device_id then
        win = joy.window.create("audio", 960, 720, 0)
        ren = joy.r2d.create(win)
        device_id = joy.audio.open_device()
        console.log("audio device: " .. device_id)

        local cx, y0, bw, bh, gap = 480, 100, 200, 60, 20
        buttons = {
            { x = cx - bw / 2, y = y0, w = bw, h = bh, label = "WAV",  r = 60,  g = 120, b = 220,
              action = function()
                  local ok = joy.audio.create("joy_beat/sounds/ak47.wav", device_id)
                  console.log("WAV create: " .. tostring(ok))
              end },
            { x = cx - bw / 2, y = y0 + bh + gap, w = bw, h = bh, label = "OGG",  r = 60,  g = 200, b = 100,
              action = function()
                  local ok = joy.audio.create("joy_beat/sounds/zombie.ogg", device_id)
                  console.log("OGG create: " .. tostring(ok))
              end },
            { x = cx - bw / 2, y = y0 + (bh + gap) * 2, w = bw, h = bh, label = "Shot", r = 220, g = 120, b = 60,
              action = function()
                  if shot then joy.audio.shot_destroy(shot) end
                  shot = joy.audio.create_shot("joy_beat/sounds/ak47.wav", device_id)
                  joy.audio.shot_play(shot)
                  console.log("shot played")
              end },
            { x = cx - bw / 2, y = y0 + (bh + gap) * 3, w = bw, h = bh, label = "BGM",  r = 180, g = 60,  b = 180,
              action = function()
                  if bgm then joy.audio.bgm_destroy(bgm) end
                  bgm = joy.audio.create_bgm("joy_beat/sounds/zombie.ogg", device_id)
                  joy.audio.bgm_set_gain(bgm, 0.5)
                  console.log("BGM started, gain=0.5")
              end },
            { x = cx - bw / 2, y = y0 + (bh + gap) * 4, w = bw, h = bh, label = "Stop", r = 200, g = 60,  b = 60,
              action = function()
                  if bgm then joy.audio.bgm_stop(bgm); console.log("BGM stopped") end
              end },
        }
    end
    set_env("audio_win", win)
    set_env("audio_ren", ren)
    set_env("audio_dev", device_id)
end

function joy.hotfix()
    win = get_env("audio_win")
    ren = get_env("audio_ren")
    device_id = get_env("audio_dev")
end

function joy.draw()
    joy.r2d.set_color(ren, 40, 40, 40, 255)
    joy.r2d.begin(ren)
    for _, btn in ipairs(buttons) do
        joy.r2d.set_color(ren, btn.r, btn.g, btn.b, 255)
        joy.r2d.rectangle(ren, "fill", btn.x, btn.y, btn.w, btn.h)
        joy.r2d.set_color(ren, 255, 255, 255, 80)
        joy.r2d.rectangle(ren, "line", btn.x, btn.y, btn.w, btn.h)
    end
    joy.r2d.end_frame(ren)
end

function joy.mousepressed(x, y, button)
    for _, btn in ipairs(buttons) do
        if x >= btn.x and x <= btn.x + btn.w and y >= btn.y and y <= btn.y + btn.h then
            btn.action()
            break
        end
    end
end

function joy.keypressed(key, scancode, repeat_, ctrl, shift)
    if key == "q" or key == "escape" then joy.event.quit() end
end

function joy.quit()
    if shot then joy.audio.shot_destroy(shot) end
    if bgm then joy.audio.bgm_destroy(bgm) end
    if device_id then joy.audio.close_device(device_id) end
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
    console.log("audio demo exit")
end

-- audio test
local win, ren, device, shot, bgm

function joy.load()
    win = joy.window.create("audio", 640, 480, 0)
    ren = joy.r2d.create(win)
    device = joy.audio.open_device()
    shot = joy.audio.create_shot(device, "joy_beat/effects/explosions.wav")
    bgm = joy.audio.create_bgm("joy_beat/effects/forest.ogg")
    joy.audio.bgm_set_volume(bgm, 0.3)
    set_env("audio_device", device)
    set_env("audio_shot", shot)
    set_env("audio_bgm", bgm)
    set_env("audio_win", win)
    set_env("audio_ren", ren)
    console.log("audio device opened")
end

function joy.update(dt)
end

function joy.draw()
    joy.r2d.begin(ren, 40, 40, 40, 255)
    joy.r2d.set_color(ren, 255, 255, 255, 255)
    joy.r2d.print(ren, nil, "F1=play shot  F2=toggle bgm  Q=quit", 20, 20, 255, 255, 255, 255)
    joy.r2d.end_frame(ren)
end

function joy.hotfix()
    win = get_env("audio_win")
    ren = get_env("audio_ren")
    device = get_env("audio_device")
    shot = get_env("audio_shot")
    bgm = get_env("audio_bgm")
end

function joy.keypressed(key)
    if key == "f1" and shot then joy.audio.shot_play(shot) end
    if key == "f2" and bgm then
        if joy.audio.bgm_is_playing(bgm) then joy.audio.bgm_stop(bgm)
        else joy.audio.bgm_play(bgm) end
    end
    if key == "q" or key == "escape" then joy.event.quit() end
end

function joy.quit()
    if not get_env("audio_device") then
        joy.audio.shot_destroy(shot)
        joy.audio.bgm_destroy(bgm)
        joy.audio.close_device(device)
    end
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

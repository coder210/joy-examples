-- 骑士动画状态机演示 (warrior_blue.png)
-- WASD=移动  Space=攻击  Q=退出
local win, ren, anim

local TEX = "joy_beat/textures/knights/troops/warrior/warrior_blue.png"
local FW, FH = 192, 192
local FRAMES = 6

local dir = 0
local ATK_ROWS = { [0] = 4, [1] = 2, [2] = 2, [3] = 6 }
local ATK_NAMES = { [0] = "attack_d", [1] = "attack_l", [2] = "attack_r", [3] = "attack_u" }

local function is_attack(s)
    return s == "attack_d" or s == "attack_l" or s == "attack_r" or s == "attack_u"
end

function joy.load()
    win = joy.window.create("animator", 800, 600, 0)
    ren = joy.r2d.create(win)
    anim = joy.animator.create(ren)
    set_env("anim_win", win)
    set_env("anim_ren", ren)
    set_env("anim_anim", anim)
        joy.animator.set_float(anim, "speed", 0)
    joy.animator.set_float(anim, "dir", 0)

    -- Idle (行0, 循环)
    joy.animator.add_state(anim, "idle")
    for col = 0, FRAMES - 1 do
        joy.animator.state_add_clip(anim, "idle", TEX, 0.15,
            { x = col * FW, y = 0, w = FW, h = FH })
    end
    joy.animator.state_set_loop(anim, "idle", true)

    -- Walk (行1, 循环)
    joy.animator.add_state(anim, "walk")
    for col = 0, FRAMES - 1 do
        joy.animator.state_add_clip(anim, "walk", TEX, 0.12,
            { x = col * FW, y = 1 * FH, w = FW, h = FH })
    end
    joy.animator.state_set_loop(anim, "walk", true)

    -- 4方向攻击 (非循环, exit_time令其播完才回idle)
    for d = 0, 3 do
        joy.animator.add_state(anim, ATK_NAMES[d])
        for col = 0, FRAMES - 1 do
            joy.animator.state_add_clip(anim, ATK_NAMES[d], TEX, 0.08,
                { x = col * FW, y = ATK_ROWS[d] * FH, w = FW, h = FH })
        end
        joy.animator.state_set_loop(anim, ATK_NAMES[d], false)
        local t = joy.animator.add_transition(anim, ATK_NAMES[d], "idle")
        joy.animator.transition_set_exit_time(anim, t, 0.08 * FRAMES)  -- 0.48秒后才能回idle
    end

    joy.animator.set_default_state(anim, "idle")

    -- idle ↔ walk (speed 条件)
    local t = joy.animator.add_transition(anim, "idle", "walk")
    joy.animator.transition_add_cond(anim, t, "speed", 0, 0.1)
    t = joy.animator.add_transition(anim, "walk", "idle")
    joy.animator.transition_add_cond(anim, t, "speed", 1, 0.1)

    -- Any State → 攻击 (trigger + dir)
    for d = 0, 3 do
        t = joy.animator.add_any_transition(anim, ATK_NAMES[d])
        joy.animator.transition_add_cond(anim, t, "atk_trig", 6, 0)
        joy.animator.transition_add_cond(anim, t, "dir", 2, d)
    end

    console.log("WASD移动 Space攻击 Q退出")
end

function joy.hotfix()
    win = get_env("anim_win")
    ren = get_env("anim_ren")
    anim = get_env("anim_anim")
end

function joy.update(dt)
    local cur = joy.animator.current_state(anim)

    -- 攻击结束后恢复移动控制
    if is_attack(cur) then
        local spd = joy.animator.get_float(anim, "speed")
        -- 攻击中不处理移动输入,攻击结束自动回idle(exit_time到期后)
    end

    joy.animator.update(anim, dt)
end

function joy.draw()
    joy.r2d.begin(ren, 40, 40, 50, 255)
    joy.animator.draw(anim, 280, 180)
    joy.r2d.end_frame(ren)
end

function joy.keypressed(key)
    if key == "escape" or key == "q" then
        joy.event.quit()
    elseif key == "space" then
        local cur = joy.animator.current_state(anim)
        if not is_attack(cur) then
            joy.animator.set_trigger(anim, "atk_trig")
        end
    elseif key == "w" then dir = 3; joy.animator.set_float(anim, "speed", 1); joy.animator.set_float(anim, "dir", 3); joy.animator.set_flip_h(anim, false)
    elseif key == "s" then dir = 0; joy.animator.set_float(anim, "speed", 1); joy.animator.set_float(anim, "dir", 0); joy.animator.set_flip_h(anim, false)
    elseif key == "a" then dir = 1; joy.animator.set_float(anim, "speed", 1); joy.animator.set_float(anim, "dir", 1); joy.animator.set_flip_h(anim, true)
    elseif key == "d" then dir = 2; joy.animator.set_float(anim, "speed", 1); joy.animator.set_float(anim, "dir", 2); joy.animator.set_flip_h(anim, false)
    end
end

function joy.keyreleased(key)
    if key == "w" or key == "s" or key == "a" or key == "d" then
        local cur = joy.animator.current_state(anim)
        if not is_attack(cur) then
            joy.animator.set_float(anim, "speed", 0)
        end
    end
end

function joy.hotfix()
    if anim then set_env("anim_anim", anim) end
    if ren then set_env("anim_ren", ren) end
    if win then set_env("anim_win", win) end
end

function joy.quit()
    if anim then joy.animator.destroy(anim) end
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

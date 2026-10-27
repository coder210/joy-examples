-- 行为树 demo（完整功能展示）
-- 演示：Sequence / Selector / Condition / Action /
--        Wait / Repeater / Inverter / Succeeder / Parallel
local win, ren, font
local bt_root
local elapsed = 0
local bt_state = "idle"

local ai = {
    hp = 100,
    target_x = 380, target_y = 300,
    my_x = 200, my_y = 300,
    speed = 150,
}

function joy.load()
    win = joy.window.create("behavior-tree", 800, 600, 0)
    ren = joy.r2d.create(win)
    font = joy.font.create("joy_beat/fonts/simhei.ttf", 18)
    set_env("bt_win", win)
    set_env("bt_ren", ren)
    set_env("bt_font", font)

    local bt = joy.behavior

    bt_root = bt.selector({
        -- 1. 逃跑
        bt.sequence({
            bt.condition(function() return ai.hp < 30 end),
            bt.action(function(dt)
                bt_state = "FLEE"
                ai.my_x = ai.my_x + 150 * dt
                ai.hp = ai.hp + 8 * dt
                return "running"
            end),
        }),
        -- 2. 巡逻→追击→攻击
        bt.sequence({
            bt.action(function(dt)
                bt_state = "PATROL"
                elapsed = elapsed + dt
                ai.my_x = ai.my_x + math.sin(elapsed) * 80 * dt
                ai.my_y = ai.my_y + math.cos(elapsed * 0.7) * 80 * dt
                local dx = ai.target_x - ai.my_x
                local dy = ai.target_y - ai.my_y
                if math.sqrt(dx * dx + dy * dy) < 200 then return "success" end
                return "running"
            end),
            bt.action(function(dt)
                bt_state = "CHASE"
                local dx = ai.target_x - ai.my_x
                local dy = ai.target_y - ai.my_y
                local dist = math.sqrt(dx * dx + dy * dy)
                if dist > 350 then return "failure" end
                ai.my_x = ai.my_x + (dx / dist) * ai.speed * dt
                ai.my_y = ai.my_y + (dy / dist) * ai.speed * dt
                if dist < 40 then return "success" end
                return "running"
            end),
            bt.wait(0.3),
            bt.repeater(bt.action(function(dt)
                local dx = ai.target_x - ai.my_x
                local dy = ai.target_y - ai.my_y
                if math.sqrt(dx * dx + dy * dy) > 80 then
                    return "failure"
                end
                bt_state = "ATTACK"
                ai.hp = ai.hp - 10 * dt
                if ai.hp <= 0 then
                    ai.hp = 100
                    console.log("target defeated!")
                    return "success"
                end
                return "running"
            end), 4),
        }),
    })
end

function joy.hotfix()
    win = get_env("bt_win")
    ren = get_env("bt_ren")
    font = get_env("bt_font")
end

function joy.update(dt)
    if bt_root then
        local s = bt_root:run(dt)
        if s ~= "running" then bt_state = "IDLE" end
    end
end

function joy.draw()
    joy.r2d.begin(ren, 30, 30, 40, 255)

    -- AI 蓝色
    joy.r2d.set_color(ren, 100, 200, 255, 255)
    joy.r2d.circle(ren, "fill", ai.my_x, ai.my_y, 20, 32)

    -- 目标红色
    joy.r2d.set_color(ren, 255, 100, 100, 255)
    joy.r2d.circle(ren, "fill", ai.target_x, ai.target_y, 10, 32)

    -- 检测圈
    joy.r2d.set_color(ren, 200, 200, 80, 60)
    joy.r2d.circle(ren, "line", ai.my_x, ai.my_y, 200, 48)

    local dx = ai.target_x - ai.my_x
    local dy = ai.target_y - ai.my_y
    local dist = math.sqrt(dx * dx + dy * dy)

    -- 信息
    joy.r2d.set_color(ren, 255, 255, 255, 255)
    joy.r2d.print(ren, font, "state: " .. bt_state, 10, 20, 255, 255, 255, 255)
    joy.r2d.print(ren, font, "hp: " .. math.floor(ai.hp), 10, 45, 255, 255, 255, 255)
    joy.r2d.print(ren, font, "dist: " .. math.floor(dist), 10, 70, 200, 200, 255, 255)
    joy.r2d.print(ren, font, "WASD=移动目标  SPACE=攻击  Q=退出", 10, 550, 180, 180, 180, 255)

    joy.r2d.end_frame(ren)
end

function joy.keypressed(key)
    if key == "escape" or key == "q" then
        joy.event.quit()
    elseif key == "space" then
        ai.hp = ai.hp - 15
    elseif key == "w" then ai.target_y = ai.target_y - 20
    elseif key == "s" then ai.target_y = ai.target_y + 20
    elseif key == "a" then ai.target_x = ai.target_x - 20
    elseif key == "d" then ai.target_x = ai.target_x + 20
    end
end

function joy.quit()
    if font then joy.r2d.destroy_font(font); font = nil end
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

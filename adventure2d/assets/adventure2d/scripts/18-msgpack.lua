-- msgpack 测试
-- 演示 joy.msgpack.pack / unpack 的基本用法
local win, ren, font

function joy.load()
    win = joy.window.create("msgpack-test", 800, 600, 0)
    ren = joy.r2d.create(win)
    font = joy.r2d.create_font(ren, "joy_beat/fonts/simhei.ttf", 18)
    set_env("msg_win", win)
    set_env("msg_ren", ren)
    set_env("msg_font", font)
end

function joy.hotfix()
    win = get_env("msg_win")
    ren = get_env("msg_ren")
    font = get_env("msg_font")
end

local results = {}
local function add(msg)
    table.insert(results, msg)
    console.log(msg)
end

-- ====== 测试开始 ======
add("=== msgpack test ===")

-- 1. 基本类型
local data = joy.msgpack.pack(nil, true, false, 42, -1, 3.14, "hello")
local vals = joy.msgpack.unpack(data)
add("1. basic: nil=" .. tostring(vals[1]) .. " true=" .. tostring(vals[2]) .. " false=" .. tostring(vals[3]))
add("   42=" .. vals[4] .. " -1=" .. vals[5] .. " pi=" .. vals[6] .. " str=" .. vals[7])

-- 2. array
local arr = joy.msgpack.pack({10, 20, 30})
local a = joy.msgpack.unpack(arr)
add("2. array: count=" .. #a .. " 1=" .. a[1] .. " 2=" .. a[2] .. " 3=" .. a[3])

-- 3. map (table with keys)
local map = joy.msgpack.pack({name="joy", ver=1, ok=true})
local m = joy.msgpack.unpack(map)
add("3. map: name=" .. m.name .. " ver=" .. m.ver .. " ok=" .. tostring(m.ok))

-- 4. nested
local nest = joy.msgpack.pack({1, {2, 3}, {x=10, y=20}})
local n = joy.msgpack.unpack(nest)
add("4. nested: [1]=" .. n[1] .. " [2][1]=" .. n[2][1] .. " [2][2]=" .. n[2][2] .. " [3].x=" .. n[3].x .. " [3].y=" .. n[3].y)

-- 5. roundtrip: pack → unpack → re-pack → compare
local orig = {a=1, b="test", c={1,2,3}}
local bin = joy.msgpack.pack(orig)
local back = joy.msgpack.unpack(bin)
add("5. roundtrip: a=" .. back.a .. " b=" .. back.b .. " c[1]=" .. back.c[1])

-- 6. 单值直接返回
local single = joy.msgpack.unpack(joy.msgpack.pack("direct"))
add("6. single: " .. single)

-- 7. 空/错误
local empty = joy.msgpack.unpack("")
add("7. empty: " .. tostring(empty))
local bad = joy.msgpack.unpack("\xff\xff")
add("   bad: " .. tostring(bad))

add("=== done ===")

function joy.draw()
    joy.r2d.begin(ren, 25, 30, 40, 255)
    joy.r2d.set_color(ren, 200, 220, 255, 255)
    local y = 20
    for i = 1, #results do
        joy.r2d.print(ren, font, results[i], 15, y, 200, 220, 255, 255)
        y = y + 22
    end
    joy.r2d.end_frame(ren)
end

function joy.keypressed(key)
    if key == "escape" or key == "q" then joy.event.quit() end
end

function joy.quit()
    if font then joy.r2d.destroy_font(font); font = nil end
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

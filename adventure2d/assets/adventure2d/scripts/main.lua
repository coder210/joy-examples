-- Demo selector for joy_beat
-- Usage: set demo_name below to one of the demo filenames (without .lua)
-- or pass as arg to joy_beat

-- Pick which demo to run:
local demo_name = ...

if not demo_name or demo_name == "" then
    demo_name = "00-hello"  -- default
end

local ok, err = pcall(function()
    local path = "demos/" .. demo_name .. ".lua"
    dofile(path)
end)

if not ok then
    console.log("Failed to load demo '" .. demo_name .. "': " .. tostring(err))
    console.log("Available demos: 00-hello, 01-crypt, 02-test-ecs, 03-kcp-client, 04-kcp-server,")
    console.log("  05-collision-circles, 06-collision-polygons, 07-collision-ray-quad,")
    console.log("  08-collision-ray-rectangle, 09-collision-rectangles, 10-graphics,")
    console.log("  11-audio, 12-ui")

    -- Provide no-op lifecycle so joy_beat doesn't error
    function joy.load() end
    function joy.update(dt) end
    function joy.draw() end
end

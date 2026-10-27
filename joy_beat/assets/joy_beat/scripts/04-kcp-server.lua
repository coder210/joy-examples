-- KCP reliable UDP server demo
local kcpserver

function joy.load()
    kcpserver = get_env("kcpserver")
    if not kcpserver then
        console.log("kcp_server test")
        kcpserver = joy.net.kcpserver.create("127.0.0.1", 30000)
    end
end

function joy.update(dt)
    if not kcpserver then return end
    joy.net.kcpserver.update(kcpserver)
    local result, msg = joy.net.kcpserver.poll(kcpserver)
    if result then
        console.log(msg.type .. ":" .. msg.conv)
        if msg.type == 3 then
            console.log(msg.type .. ":" .. msg.conv .. ":" .. msg.data)
            joy.net.kcpserver.send(kcpserver, msg.conv, "hello from server")
        end
    end
end

function joy.draw()
end

function joy.hotfix()
    if kcpserver then set_env("kcpserver", kcpserver) end
end

function joy.quit()
    if kcpserver then joy.net.kcpserver.destroy(kcpserver) end
end

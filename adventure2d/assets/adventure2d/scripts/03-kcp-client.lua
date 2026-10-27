-- KCP reliable UDP client demo
local kcpclient

function joy.load()
    kcpclient = get_env("kcpclient")
    if not kcpclient then
        console.log("kcp_client test")
        kcpclient = joy.net.kcpclient.create("127.0.0.1", 30000)
    end
end

function joy.update(dt)
    if not kcpclient then return end
    joy.net.kcpclient.update(kcpclient)
    local result, msg = joy.net.kcpclient.poll(kcpclient)
    if result then
        console.log(msg.type .. ":" .. msg.conv)
        if msg.type == 1 then
            joy.net.kcpclient.send(kcpclient, "hello from client")
        elseif msg.type == 3 then
            console.log(msg.type .. ":" .. msg.conv .. ":" .. msg.data)
        end
    end
end

function joy.draw()
end

function joy.hotfix()
    if kcpclient then set_env("kcpclient", kcpclient) end
end

function joy.quit()
    if kcpclient then joy.net.kcpclient.destroy(kcpclient) end
end

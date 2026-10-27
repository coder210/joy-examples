-- 线程工作脚本：使用 joy 生命周期（类似主线程）
-- 启动后会进入死循环，持续调用 joy.update()

function joy.load()
    joy.sdl.log("worker: load")
end

function joy.update(dt)
    --joy.sdl.log("worker: update")
end

function joy.quit()
    joy.sdl.log("worker: quit")
end
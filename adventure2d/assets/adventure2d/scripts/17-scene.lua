-- 场景系统 + UI 控件演示
local win, ren, font
local mgr, menu_scene, menu_root_node
local btn_start, btn_quit, label_title
local game_btn_back = nil  -- 当前游戏场景的返回按钮
local search_result = ""   -- 查找测试结果显示

function joy.load()
    win = joy.window.create("scene-demo", 800, 600, 0)
    ren = joy.r2d.create(win)
    font = joy.r2d.create_font(ren, "joy_beat/fonts/simhei.ttf", 24)
    local s = joy.scene
    mgr = s.mgr_create()
    set_env("sc_win", win)
    set_env("sc_ren", ren)
    set_env("sc_mgr", mgr)

    -- 菜单场景
    menu_scene = s.create("menu")

    label_title = joy.ui.label.create(ren, font, {27426, 26223, 31649, 26223})
    joy.ui.label.set_position(label_title, 320, 80)

    btn_start = joy.ui.button.create(ren, 200, 50)
    joy.ui.button.set_position(btn_start, 300, 220)
    joy.ui.button.set_text(btn_start, font, {24320, 22987, 28216, 25195}, 4, 255, 255, 255, 255)

    btn_quit = joy.ui.button.create(ren, 200, 50)
    joy.ui.button.set_position(btn_quit, 300, 300)
    joy.ui.button.set_text(btn_quit, font, {36864, 20986}, 4, 255, 255, 255, 255)

    -- 节点树（带名称和标签，用于查找测试）
    menu_root_node = s.node_create()
    s.node_set_name(menu_root_node, "tree_root")
    s.node_set_tag(menu_root_node, "root")
    s.node_set_position(menu_root_node, 400, 460)
    local c1 = s.node_create(); s.node_set_position(c1, -50, 50); s.node_add_child(menu_root_node, c1)
    s.node_set_name(c1, "left_branch"); s.node_set_tag(c1, "branch")
    local c2 = s.node_create(); s.node_set_position(c2, 50, 50);  s.node_add_child(menu_root_node, c2)
    s.node_set_name(c2, "right_branch"); s.node_set_tag(c2, "branch")
    s.add_root(menu_scene, menu_root_node)

    s.mgr_push(mgr, menu_scene)
end

local function draw_node(n, ox, oy)
    if not n then return end
    local s = joy.scene
    local x, y = s.node_get_position(n)
    local wx, wy = ox + x, oy + y
    if ox ~= 0 or oy ~= 0 then
        joy.r2d.set_color(ren, 80, 80, 120, 180)
        joy.r2d.line(ren, ox, oy, wx, wy)
    end
    joy.r2d.set_color(ren, 120, 200, 255, 220)
    joy.r2d.circle(ren, "fill", wx, wy, 6, 10)
    local child = s.node_get_child(n)
    while child do draw_node(child, wx, wy); child = s.node_get_sibling(child) end
end

function joy.hotfix()
    win = get_env("sc_win")
    ren = get_env("sc_ren")
    mgr = get_env("sc_mgr")
end

function joy.draw()
    joy.r2d.begin(ren, 40, 45, 60, 255)
    local s = joy.scene
    local cur = s.mgr_current(mgr)
    local name = cur and s.get_name(cur) or "none"

    if name == "menu" then
        joy.ui.label.draw(label_title)
        joy.ui.button.draw(btn_start)
        joy.ui.button.draw(btn_quit)
        if menu_root_node then draw_node(menu_root_node, 400, 460) end
    elseif name == "game" then
        joy.r2d.set_color(ren, 100, 200, 255, 255)
        joy.r2d.circle(ren, "fill", 400, 300, 60, 32)
        joy.r2d.set_color(ren, 255, 255, 255, 255)
        joy.r2d.print(ren, font, "Game Scene", 340, 280, 255, 255, 255, 255)
        if game_btn_back then joy.ui.button.draw(game_btn_back) end
    end

    joy.r2d.set_color(ren, 200, 200, 200, 255)
    joy.r2d.print(ren, font, "scene: " .. name, 10, 550, 200, 200, 200, 255)
    if #search_result > 0 then
        joy.r2d.set_color(ren, 255, 255, 100, 255)
        joy.r2d.print(ren, font, search_result, 10, 515, 255, 255, 100, 255)
    end
    joy.r2d.end_frame(ren)
end

function joy.mousepressed(x, y, button)
    local cur = joy.scene.mgr_current(mgr)
    local name = cur and joy.scene.get_name(cur) or ""
    if name == "menu" then
        if joy.ui.button.is_pressed(btn_start, x, y) then
            joy.ui.button.set_pressed(btn_start, false)
            console.log("-> game")
            local s = joy.scene
            local gs = s.create("game")
            game_btn_back = joy.ui.button.create(ren, 120, 40)
            joy.ui.button.set_position(game_btn_back, 20, 20)
            joy.ui.button.set_text(game_btn_back, font, {36864, 22238}, 4, 255, 255, 255, 255)
            s.add_root(gs, s.node_create())
            s.mgr_push(mgr, gs)
        elseif joy.ui.button.is_pressed(btn_quit, x, y) then
            joy.ui.button.set_pressed(btn_quit, false)
            joy.event.quit()
        end
    elseif name == "game" then
        if game_btn_back and joy.ui.button.is_pressed(game_btn_back, x, y) then
            joy.ui.button.set_pressed(game_btn_back, false)
            console.log("-> menu")
            game_btn_back = nil
            joy.scene.mgr_pop(mgr)
        end
    end
end

function joy.keypressed(key)
    if key == "escape" or key == "q" then joy.event.quit() end

    local cur = joy.scene.mgr_current(mgr)
    local cur_name = cur and joy.scene.get_name(cur) or ""
    if cur_name ~= "menu" then return end  -- 以下查找测试仅在菜单场景有效

    local s = joy.scene
    if key == "1" then
        -- 按名称查找
        local found = s.find_node_by_name(cur, "left_branch")
        if found then
            local x, y = s.node_get_position(found)
            local tag = s.node_get_tag(found)
            search_result = "find[left_branch] pos=(" .. math.floor(x) .. "," .. math.floor(y) .. ") tag=" .. tag
        else
            search_result = "find[left_branch] = nil"
        end
    elseif key == "2" then
        -- 按名称查找（不存在的）
        local found = s.find_node_by_name(cur, "ghost")
        search_result = "find[ghost] = " .. tostring(found)
    elseif key == "3" then
        -- 按标签查找
        local nodes = s.find_nodes_by_tag(cur, "branch")
        local msg = "tag[branch] count=" .. #nodes
        for i = 1, #nodes do
            msg = msg .. "  " .. i .. "=" .. s.node_get_name(nodes[i])
        end
        search_result = msg
    elseif key == "4" then
        -- manager 快捷查找（返回 table）
        local found = s.mgr_find_node(mgr, "tree_root")
        if found then
            search_result = "mgr_find[tree_root] tag=" .. found.tag .. " children=" .. found.children
        else
            search_result = "mgr_find[tree_root] = nil"
        end
    end
end

function joy.quit()
    if font then joy.r2d.destroy_font(font) end
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

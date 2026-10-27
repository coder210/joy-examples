state = state or {}
local win

function joy.load()
    win = joy.window.create("test", 800, 600, 0)
    ren = joy.r2d.create(win)
    font = joy.font.create("joy_beat/fonts/simhei.ttf", 24)

    -- label
    state.label = joy.ui.label.create(ren, font, {27979, 26412, 35797, 33021})
    joy.ui.label.set_position(state.label, 10, 5)

    -- textinput
    state.input = joy.ui.textinput.create(ren, font, 10, 35, 250, 30)
    joy.ui.textinput.set_text(state.input, "hello jui")

    -- buttons
    state.btn = joy.ui.button.create(ren, 120, 30)
    joy.ui.button.set_position(state.btn, 10, 80)
    joy.ui.button.set_text(state.btn, font, {26080, 20986, 25105}, 4, 255, 255, 255, 255)

    state.btn2 = joy.ui.button.create(ren, 120, 30)
    joy.ui.button.set_position(state.btn2, 150, 80)
    joy.ui.button.set_text(state.btn2, font, {25353, 20986, 25105}, 4, 255, 255, 255, 255)

    -- checkbox
    state.cb = joy.ui.checkbox.create(10, 130, 16, "cb")

    -- combobox
    state.cb2 = joy.ui.combobox.create(ren, 120, 125, 140, 28)
    joy.ui.combobox.additem(state.cb2, "item 1", font)
    joy.ui.combobox.additem(state.cb2, "item 2", font)
    joy.ui.combobox.additem(state.cb2, "item 3", font)

    -- datagrid
    state.dg = joy.ui.datagrid.create(ren, {x=300, y=5, w=480, h=400}, 4, true, font)
    joy.ui.datagrid.setheaders(state.dg, font, {"ID", "Name", "Score", "Level"})
    joy.ui.datagrid.addrow(state.dg, {"1", "Alice", "95", "10"})
    joy.ui.datagrid.addrow(state.dg, {"2", "Bob", "87", "8"})
    joy.ui.datagrid.addrow(state.dg, {"3", "Charlie", "92", "9"})
    joy.ui.datagrid.addrow(state.dg, {"4", "Diana", "78", "7"})

    -- slider
    state.slider = joy.ui.slider.create(ren, 10, 170, 200, 16, 0, 100, 50)

    -- popup
    state.popup = joy.ui.popup.create(ren, 350, 220)
    joy.ui.popup.set_title(state.popup, "Popup Demo")
    joy.ui.popup.set_content(state.popup, "This is a popup dialog.\nYou can drag it by the title bar.\nPress P to toggle visibility.\nClose with the X button or press P again.")
    joy.ui.popup.set_position(state.popup, 225, 170)
    state.popup_visible = true

    -- tooltip（快速创建，跟随鼠标）
    state.tip = joy.ui.tooltip.create(ren, "hover me!")
    state.tip_text = ""

    state.log = ""
end

function joy.hotfix()
    win = get_env("ui_win")
    ren = get_env("ui_ren")
    font = get_env("ui_font")
end

function joy.update(dt)
end

function joy.draw()
    joy.r2d.begin(ren, 50, 50, 50, 255)

    joy.ui.label.draw(state.label)
    joy.ui.textinput.draw(state.input)
    joy.ui.button.draw(state.btn)
    joy.ui.button.draw(state.btn2)
    joy.ui.checkbox.draw(state.cb, ren)
    joy.ui.combobox.draw(state.cb2)
    joy.ui.datagrid.draw(state.dg, ren)
    joy.ui.slider.draw(state.slider)

    joy.r2d.set_color(ren, 255, 255, 255, 255)
    joy.r2d.print(ren, font, state.log, 10, 420, 255, 255, 255, 255)
    joy.r2d.print(ren, font, "text: " .. joy.ui.textinput.get_text(state.input), 10, 450, 255, 255, 255, 200)
    joy.r2d.print(ren, font, "rows: " .. joy.ui.datagrid.get_row_count(state.dg), 10, 480, 200, 255, 255, 255)
    joy.r2d.print(ren, font, "slider: " .. math.floor(joy.ui.slider.get_value(state.slider) + 0.5), 10, 510, 255, 255, 200, 255)

    joy.ui.combobox.draw(state.cb2)

    -- popup
    if state.popup_visible then
        joy.ui.popup.draw(state.popup, font, font)
    end

    -- tooltip
    if #state.tip_text > 0 then
        joy.ui.tooltip.set_text(state.tip, state.tip_text)
        joy.ui.tooltip.draw(state.tip, font)
    end

    joy.r2d.end_frame(ren)
end

function joy.mousepressed(x, y, button)
    -- 弹窗模态：显示时外部点击不处理
    if state.popup_visible and not joy.ui.popup.is_inside(state.popup, x, y) then
        return
    end

    if joy.ui.button.is_pressed(state.btn, x, y) then
        state.log = "btn1 pressed!"
    elseif joy.ui.button.is_pressed(state.btn2, x, y) then
        state.log = "btn2 pressed!"
    end
    joy.ui.textinput.is_pressed(state.input, x, y)
    joy.ui.checkbox.is_pressed(state.cb, x, y)

    -- combobox
    if joy.ui.combobox.is_pressed(state.cb2, x, y) then
        joy.ui.combobox.set_open(state.cb2, not joy.ui.combobox.is_open(state.cb2))
    else
        local hit = joy.ui.combobox.hit_item(state.cb2, x, y)
        if hit >= 0 then
            joy.ui.combobox.set_selected(state.cb2, hit)
            joy.ui.combobox.set_open(state.cb2, false)
            state.log = "combo: " .. hit
        elseif joy.ui.combobox.is_open(state.cb2) then
            joy.ui.combobox.set_open(state.cb2, false)
        end
    end

    -- datagrid
    local hit_row = joy.ui.datagrid.hit_row(state.dg, x, y)
    if hit_row >= 0 then
        joy.ui.datagrid.set_selected_row(state.dg, hit_row)
        local name = joy.ui.datagrid.get_cell(state.dg, hit_row, 1)
        state.log = "row " .. hit_row .. ": " .. (name or "?")
    end

    -- slider
    if joy.ui.slider.is_pressed(state.slider, x, y) then
        state.log = "slider: " .. math.floor(joy.ui.slider.get_value(state.slider) + 0.5)
    end

    -- popup
    if state.popup_visible then
        if joy.ui.popup.is_close_clicked(state.popup, x, y) then
            state.popup_visible = false
            state.log = "popup closed"
        elseif joy.ui.popup.is_title_clicked(state.popup, x, y) then
            state.popup_dragging = true
        end
    end
end

function joy.mousereleased(x, y, button)
    joy.ui.button.set_pressed(state.btn, false)
    joy.ui.button.set_pressed(state.btn2, false)
    joy.ui.slider.set_pressed(state.slider, false)
    state.popup_dragging = false
end

function joy.mousemoved(x, y, dx, dy)
    joy.ui.button.is_hovered(state.btn, x, y)
    joy.ui.button.is_hovered(state.btn2, x, y)
    joy.ui.textinput.is_hovered(state.input, x, y)
    joy.ui.slider.is_hovered(state.slider, x, y)
    joy.ui.slider.drag_move(state.slider, x, y)
    if state.popup_dragging then
        joy.ui.popup.drag_move(state.popup, x, y)
    end
    -- tooltip 跟随鼠标
    joy.ui.tooltip.set_position(state.tip, x + 12, y - 10)
    if joy.ui.button.is_hovered(state.btn, x, y) then
        state.tip_text = "Button 1"
    elseif joy.ui.button.is_hovered(state.btn2, x, y) then
        state.tip_text = "Button 2"
    elseif joy.ui.slider.is_hovered(state.slider, x, y) then
        state.tip_text = "Slider: " .. math.floor(joy.ui.slider.get_value(state.slider) + 0.5)
    elseif joy.ui.textinput.is_hovered(state.input, x, y) then
        state.tip_text = "Text Input"
    else
        state.tip_text = ""
    end
end

function joy.textinput(text)
    joy.ui.textinput.insert(state.input, text)
end

function joy.keypressed(key, scancode, repeat_, ctrl, shift)
    if key == "escape" then joy.event.quit()
    elseif key == "backspace" then joy.ui.textinput.backspace(state.input)
    elseif key == "delete" then joy.ui.textinput.delete(state.input)
    elseif key == "left" then
        if shift then joy.ui.textinput.select_left(state.input)
        else joy.ui.textinput.cursor_left(state.input) end
    elseif key == "right" then
        if shift then joy.ui.textinput.select_right(state.input)
        else joy.ui.textinput.cursor_right(state.input) end
    elseif key == "home" then
        if shift then joy.ui.textinput.select_home(state.input)
        else joy.ui.textinput.cursor_home(state.input) end
    elseif key == "end" then
        if shift then joy.ui.textinput.select_end(state.input)
        else joy.ui.textinput.cursor_end(state.input) end
    elseif key == "return" then
        if joy.ui.textinput.is_focused(state.input) then
            state.log = "submit: " .. joy.ui.textinput.get_text(state.input)
        end
    elseif key == "p" then
        state.popup_visible = not state.popup_visible
        state.log = state.popup_visible and "popup shown" or "popup hidden"
    elseif ctrl then
        if key == "a" then joy.ui.textinput.select_all(state.input)
        elseif key == "v" then joy.ui.textinput.paste(state.input)
        elseif key == "c" then joy.ui.textinput.copy(state.input)
        elseif key == "x" then joy.ui.textinput.cut(state.input)
        end
    end
end

function joy.quit()
    if font then joy.r2d.destroy_font(font); font = nil end
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

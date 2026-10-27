local win, ren, font

function joy.load()
    win = get_env("ui_win")
    ren = get_env("ui_ren")
    font = get_env("ui_font")
    if not font then
        win = joy.window.create("test", 800, 600, 0)
        ren = joy.r2d.create(win)
        font = joy.font.create("joy_beat/fonts/simhei.ttf", 24)
        set_env("ui_win", win)
        set_env("ui_ren", ren)
        set_env("ui_font", font)
    end

    set_env("ui_label", joy.ui.label.create(ren, font))
    joy.ui.label.set_position(get_env("ui_label"), 10, 5)

    set_env("ui_input", joy.ui.inputbox.create(ren, font, 10, 35, 250, 30))
    joy.ui.inputbox.set_text(get_env("ui_input"), "hello jui")

    set_env("ui_btn", joy.ui.button.create(ren, 120, 30))
    joy.ui.button.set_position(get_env("ui_btn"), 10, 80)
    joy.ui.button.set_text(get_env("ui_btn"), font, "Button1", {r=255, g=255, b=255, a=255})

    set_env("ui_btn2", joy.ui.button.create(ren, 120, 30))
    joy.ui.button.set_position(get_env("ui_btn2"), 150, 80)
    joy.ui.button.set_text(get_env("ui_btn2"), font, "Button2", {r=255, g=255, b=255, a=255})

    set_env("ui_cb", joy.ui.checkbox.create(10, 130, 16, "cb"))

    set_env("ui_cb2", joy.ui.combobox.create(ren, 120, 125, 140, 28))
    joy.ui.combobox.additem(get_env("ui_cb2"), "item 1", font)
    joy.ui.combobox.additem(get_env("ui_cb2"), "item 2", font)
    joy.ui.combobox.additem(get_env("ui_cb2"), "item 3", font)

    set_env("ui_dg", joy.ui.datagrid.create(ren, {x=300, y=5, w=480, h=400}, 4, true, font))
    joy.ui.datagrid.setheaders(get_env("ui_dg"), font, {"ID", "Name", "Score", "Level"})
    joy.ui.datagrid.addrow(get_env("ui_dg"), {"1", "Alice", "95", "10"})
    joy.ui.datagrid.addrow(get_env("ui_dg"), {"2", "Bob", "87", "8"})
    joy.ui.datagrid.addrow(get_env("ui_dg"), {"3", "Charlie", "92", "9"})
    joy.ui.datagrid.addrow(get_env("ui_dg"), {"4", "Diana", "78", "7"})

    set_env("ui_slider", joy.ui.slider.create(ren, 10, 170, 200, 16, 0, 100, 50))

    set_env("ui_popup", joy.ui.popup.create(ren, 350, 220))
    joy.ui.popup.set_title(get_env("ui_popup"), "Popup Demo")
    joy.ui.popup.set_content(get_env("ui_popup"), "This is a popup dialog.\nYou can drag it by the title bar.\nPress P to toggle visibility.\nClose with the X button or press P again.")
    joy.ui.popup.set_position(get_env("ui_popup"), 225, 170)
    set_env("ui_popup_visible", true)
    set_env("ui_popup_dragging", false)

    set_env("ui_tip", joy.ui.tooltip.create(ren, "hover me!"))
    set_env("ui_tip_text", "")

    set_env("ui_log", "")
end

function joy.hotfix()
    win = get_env("ui_win")
    ren = get_env("ui_ren")
    font = get_env("ui_font")
    joy.load()
end

function joy.update(dt)
end

function joy.draw()
    local label = get_env("ui_label")
    local input = get_env("ui_input")
    local cb2 = get_env("ui_cb2")
    if not label or not input or not cb2 then return end

    joy.r2d.begin(ren, 50, 50, 50, 255)

    joy.ui.label.draw(label)
    joy.ui.inputbox.draw(input)
    joy.ui.button.draw(get_env("ui_btn"))
    joy.ui.button.draw(get_env("ui_btn2"))
    joy.ui.checkbox.draw(get_env("ui_cb"), ren)
    joy.ui.combobox.draw(cb2)
    joy.ui.datagrid.draw(get_env("ui_dg"), ren)
    joy.ui.slider.draw(get_env("ui_slider"))

    joy.r2d.set_color(ren, 255, 255, 255, 255)
    joy.r2d.print(ren, font, get_env("ui_log") or "", 10, 420, 255, 255, 255, 255)
    joy.r2d.print(ren, font, "text: " .. joy.ui.inputbox.get_text(input), 10, 450, 255, 255, 255, 200)
    joy.r2d.print(ren, font, "rows: " .. joy.ui.datagrid.get_row_count(get_env("ui_dg")), 10, 480, 200, 255, 255, 255)
    joy.r2d.print(ren, font, "slider: " .. math.floor(joy.ui.slider.get_value(get_env("ui_slider")) + 0.5), 10, 510, 255, 255, 200, 255)

    if get_env("ui_popup_visible") then
        joy.ui.popup.draw(get_env("ui_popup"), font, font)
    end

    local tip_text = get_env("ui_tip_text") or ""
    if #tip_text > 0 then
        joy.ui.tooltip.set_text(get_env("ui_tip"), tip_text)
        joy.ui.tooltip.draw(get_env("ui_tip"), font)
    end

    joy.r2d.end_frame(ren)
end

-- C 层的 process_event 已自动处理 button/inputbox/slider/checkbox 的鼠标/键盘事件
-- 脚本只需要处理 combobox/popup/tooltip 的业务逻辑

function joy.process_event(e)
    local label = get_env("ui_label")
    local input = get_env("ui_input")
    local cb2 = get_env("ui_cb2")
    if not label or not input or not cb2 then return end

    joy.ui.label.process_event(label, e);
    joy.ui.inputbox.process_event(input, e);
    joy.ui.button.process_event(get_env("ui_btn"), e);
    joy.ui.button.process_event(get_env("ui_btn2"), e);
    joy.ui.checkbox.process_event(get_env("ui_cb"), e);
    joy.ui.slider.process_event(get_env("ui_slider"), e);
    joy.ui.radio.process_event(get_env("ui_radio"), e);
    joy.ui.combobox.process_event(cb2, e);
    joy.ui.popup.process_event(get_env("ui_popup"), e);
    joy.ui.tooltip.process_event(get_env("ui_tip"), e);
    joy.ui.datagrid.process_event(get_env("ui_dg"), e);
end

function joy.quit()
    if font then joy.r2d.destroy_font(font); font = nil end
    if ren then joy.r2d.destroy(ren); ren = nil end
    if win then joy.window.destroy(win); win = nil end
end

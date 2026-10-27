var state = state || {};
var win, ren, fnt;

joy.load = function()
{
    win = get_env("ui_win");
    ren = get_env("ui_ren");
    fnt = get_env("ui_font");
    if (!fnt) {
        win = window.create("test", 800, 600, 0);
        ren = r2d.create(win);
        fnt = font.create("joy_beat/fonts/simhei.ttf", 24);
        set_env("ui_win", win);
        set_env("ui_ren", ren);
        set_env("ui_font", fnt);
    }

    state.label = ui.label.create(ren, fnt);
    ui.label.set_position(state.label, 10, 5);

    state.input = ui.inputbox.create(ren, fnt, 10, 35, 250, 30);
    ui.inputbox.set_text(state.input, "hello jui");

    state.btn = ui.button.create(ren, 120, 30);
    ui.button.set_position(state.btn, 10, 80);
    ui.button.set_text(state.btn, fnt, "Button1", {r:255, g:255, b:255, a:255});

    state.btn2 = ui.button.create(ren, 120, 30);
    ui.button.set_position(state.btn2, 150, 80);
    ui.button.set_text(state.btn2, fnt, "Button2", {r:255, g:255, b:255, a:255});

    state.cb = ui.checkbox.create(10, 130, 16, "cb");

    state.cb2 = ui.combobox.create(ren, 120, 125, 140, 28);
    ui.combobox.additem(state.cb2, "item 1", fnt);
    ui.combobox.additem(state.cb2, "item 2", fnt);
    ui.combobox.additem(state.cb2, "item 3", fnt);

    state.dg = ui.datagrid.create(ren, 300, 5, 480, 400, 4, true, fnt);
    ui.datagrid.setheaders(state.dg, fnt, ["ID", "Name", "Score", "Level"]);
    ui.datagrid.addrow(state.dg, ["1", "Alice", "95", "10"]);
    ui.datagrid.addrow(state.dg, ["2", "Bob", "87", "8"]);
    ui.datagrid.addrow(state.dg, ["3", "Charlie", "92", "9"]);
    ui.datagrid.addrow(state.dg, ["4", "Diana", "78", "7"]);

    state.slider = ui.slider.create(ren, 10, 170, 200, 16, 0, 100, 50);

    state.popup = ui.popup.create(ren, 350, 220);
    ui.popup.set_title(state.popup, "Popup Demo");
    ui.popup.set_content(state.popup, "This is a popup dialog.\nYou can drag it by the title bar.\nPress P to toggle visibility.\nClose with the X button or press P again.");
    ui.popup.set_position(state.popup, 225, 170);
    state.popup_visible = true;
    state.popup_dragging = false;

    state.tip = ui.tooltip.create(ren, "hover me!");
    state.tip_text = "";

    state.log = "";
    // 收集所有需要 C 层自动处理事件的控件
    state.widgets = [state.btn, state.btn2, state.input, state.slider, state.cb];
};

joy.hotfix = function() {
    win = get_env("ui_win");
    ren = get_env("ui_ren");
    fnt = get_env("ui_font");
};

joy.update = function(dt) {};

joy.draw = function()
{
    r2d.begin(ren, 50, 50, 50, 255);

    ui.label.draw(state.label);
    ui.inputbox.draw(state.input);
    ui.button.draw(state.btn);
    ui.button.draw(state.btn2);
    ui.checkbox.draw(state.cb, ren);
    ui.combobox.draw(state.cb2);
    ui.datagrid.draw(state.dg, ren);
    ui.slider.draw(state.slider);

    r2d.set_color(ren, 255, 255, 255, 255);
    r2d.print(ren, fnt, state.log, 10, 420, {r:255, g:255, b:255, a:255});
    r2d.print(ren, fnt, "text: " + ui.inputbox.get_text(state.input), 10, 450, {r:255, g:255, b:255, a:200});
    r2d.print(ren, fnt, "rows: " + ui.datagrid.get_row_count(state.dg), 10, 480, {r:200, g:255, b:255, a:255});
    r2d.print(ren, fnt, "slider: " + Math.floor(ui.slider.get_value(state.slider) + 0.5), 10, 510, {r:255, g:255, b:200, a:255});

    ui.combobox.draw(state.cb2);

    if (state.popup_visible) {
        ui.popup.draw(state.popup, fnt, fnt);
    }

    if (state.tip_text.length > 0) {
        ui.tooltip.set_text(state.tip, state.tip_text);
        ui.tooltip.draw(state.tip, fnt);
    }

    r2d.end_frame(ren);
};

// ---- 统一事件处理 ----
// button/inputbox/slider/checkbox 的鼠标/键盘事件已由 C 层 process_event 自动处理
// 脚本只需要处理 combobox、popup、tooltip 的业务逻辑

joy.mousepressed = function(x, y, button)
{
    if (state.popup_visible && !ui.popup.is_inside(state.popup, x, y)) {
        return;
    }

    // 按钮业务
    if (ui.button.is_pressed(state.btn, x, y)) {
        state.log = "btn1 pressed!";
    } else if (ui.button.is_pressed(state.btn2, x, y)) {
        state.log = "btn2 pressed!";
    }

    // combobox
    if (ui.combobox.is_pressed(state.cb2, x, y)) {
        ui.combobox.set_open(state.cb2, !ui.combobox.is_open(state.cb2));
    } else {
        var hit = ui.combobox.hit_item(state.cb2, x, y);
        if (hit >= 0) {
            ui.combobox.set_selected(state.cb2, hit);
            ui.combobox.set_open(state.cb2, false);
            state.log = "combo: " + hit;
        } else if (ui.combobox.is_open(state.cb2)) {
            ui.combobox.set_open(state.cb2, false);
        }
    }

    // datagrid
    var hit_row = ui.datagrid.hit_row(state.dg, x, y);
    if (hit_row >= 0) {
        ui.datagrid.set_selected_row(state.dg, hit_row);
        var name = ui.datagrid.get_cell(state.dg, hit_row, 1);
        state.log = "row " + hit_row + ": " + (name === undefined ? "?" : name);
    }

    // popup
    if (state.popup_visible) {
        if (ui.popup.is_close_clicked(state.popup, x, y)) {
            state.popup_visible = false;
            state.log = "popup closed";
        } else if (ui.popup.is_title_clicked(state.popup, x, y)) {
            state.popup_dragging = true;
        }
    }
};

joy.mousereleased = function(x, y, button)
{
    state.popup_dragging = false;
};

joy.mousemoved = function(x, y, dx, dy)
{
    if (state.popup_dragging) {
        ui.popup.drag_move(state.popup, x, y);
    }
    ui.tooltip.set_position(state.tip, x + 12, y - 10);
    if (ui.button.is_hovered(state.btn, x, y)) {
        state.tip_text = "Button 1";
    } else if (ui.button.is_hovered(state.btn2, x, y)) {
        state.tip_text = "Button 2";
    } else if (ui.slider.is_hovered(state.slider, x, y)) {
        state.tip_text = "Slider: " + Math.floor(ui.slider.get_value(state.slider) + 0.5);
    } else if (ui.inputbox.is_hovered(state.input, x, y)) {
        state.tip_text = "Text Input";
    } else {
        state.tip_text = "";
    }
};

joy.keypressed = function(key, scancode, repeat_, ctrl, shift, alt)
{
    if (key === "escape") {
        event.quit();
    } else if (key == "return") {
        if (ui.inputbox.is_focused(state.input)) {
            state.log = "submit: " + ui.inputbox.get_text(state.input);
        }
    } else if (key == "p") {
        state.popup_visible = !state.popup_visible;
        state.log = state.popup_visible ? "popup shown" : "popup hidden";
    }
};

joy.process_event = function(e)
{
    ui.label.process_event(state.label, e);
    ui.inputbox.process_event(state.input, e);
    ui.button.process_event(state.btn, e);
    ui.button.process_event(state.btn2, e);
    ui.checkbox.process_event(state.cb, e);
    ui.slider.process_event(state.slider, e);
    ui.radio.process_event(state.radio, e);
    ui.combobox.process_event(state.cb2, e);
    ui.popup.process_event(state.popup, e);
    ui.tooltip.process_event(state.tip, e);
    ui.datagrid.process_event(state.dg, e);
    ui.joystick.process_event(state.js, e);
};

joy.quit = function()
{
    if (ren) { r2d.destroy(ren); ren = null; }
    if (win) { window.destroy(win); win = null; }
};

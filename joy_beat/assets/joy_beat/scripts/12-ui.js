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

    // label
    state.label = ui.label.create(ren, fnt);
    ui.label.set_position(state.label, 10, 5);

    // textinput
    state.input = ui.textinput.create(ren, fnt, 10, 35, 250, 30);
    ui.textinput.set_text(state.input, "hello jui");

    // buttons
    state.btn = ui.button.create(ren, 120, 30);
    ui.button.set_position(state.btn, 10, 80);
    ui.button.set_text(state.btn, fnt, "Button1", {r:255, g:255, b:255, a:255});

    state.btn2 = ui.button.create(ren, 120, 30);
    ui.button.set_position(state.btn2, 150, 80);
    ui.button.set_text(state.btn2, fnt, "Button2", {r:255, g:255, b:255, a:255});

    // checkbox
    state.cb = ui.checkbox.create(10, 130, 16, "cb");

    // combobox
    state.cb2 = ui.combobox.create(ren, 120, 125, 140, 28);
    ui.combobox.additem(state.cb2, "item 1", fnt);
    ui.combobox.additem(state.cb2, "item 2", fnt);
    ui.combobox.additem(state.cb2, "item 3", fnt);

    // datagrid
    state.dg = ui.datagrid.create(ren, 300, 5, 480, 400, 4, true, fnt);
    ui.datagrid.setheaders(state.dg, fnt, ["ID", "Name", "Score", "Level"]);
    ui.datagrid.addrow(state.dg, ["1", "Alice", "95", "10"]);
    ui.datagrid.addrow(state.dg, ["2", "Bob", "87", "8"]);
    ui.datagrid.addrow(state.dg, ["3", "Charlie", "92", "9"]);
    ui.datagrid.addrow(state.dg, ["4", "Diana", "78", "7"]);

    // slider
    state.slider = ui.slider.create(ren, 10, 170, 200, 16, 0, 100, 50);

    // popup
    state.popup = ui.popup.create(ren, 350, 220);
    ui.popup.set_title(state.popup, "Popup Demo");
    ui.popup.set_content(state.popup, "This is a popup dialog.\nYou can drag it by the title bar.\nPress P to toggle visibility.\nClose with the X button or press P again.");
    ui.popup.set_position(state.popup, 225, 170);
    state.popup_visible = true;
    state.popup_dragging = false;

    // tooltip
    state.tip = ui.tooltip.create(ren, "hover me!");
    state.tip_text = "";

    state.log = "";
};

joy.hotfix = function() {
    win = get_env("ui_win");
    ren = get_env("ui_ren");
    fnt = get_env("ui_font");
};

joy.update = function(dt)
{
};

joy.draw = function()
{
    r2d.begin(ren, 50, 50, 50, 255);

    ui.label.draw(state.label);
    ui.textinput.draw(state.input);
    ui.button.draw(state.btn);
    ui.button.draw(state.btn2);
    ui.checkbox.draw(state.cb, ren);
    ui.combobox.draw(state.cb2);
    ui.datagrid.draw(state.dg, ren);
    ui.slider.draw(state.slider);

    r2d.set_color(ren, 255, 255, 255, 255);
    r2d.print(ren, fnt, state.log, 10, 420, {r:255, g:255, b:255, a:255});
    r2d.print(ren, fnt, "text: " + ui.textinput.get_text(state.input), 10, 450, {r:255, g:255, b:255, a:200});
    r2d.print(ren, fnt, "rows: " + ui.datagrid.get_row_count(state.dg), 10, 480, {r:200, g:255, b:255, a:255});
    r2d.print(ren, fnt, "slider: " + Math.floor(ui.slider.get_value(state.slider) + 0.5), 10, 510, {r:255, g:255, b:200, a:255});

    ui.combobox.draw(state.cb2);

    // popup
    if (state.popup_visible) {
        ui.popup.draw(state.popup, fnt, fnt);
    }

    // tooltip
    if (state.tip_text.length > 0) {
        ui.tooltip.set_text(state.tip, state.tip_text);
        ui.tooltip.draw(state.tip, fnt);
    }

    r2d.end_frame(ren);
};

joy.mousepressed = function(x, y, button)
{
    // 弹窗模态：显示时外部点击不处理
    if (state.popup_visible && !ui.popup.is_inside(state.popup, x, y)) {
        return;
    }

    if (ui.button.is_pressed(state.btn, x, y)) {
        state.log = "btn1 pressed!";
    } else if (ui.button.is_pressed(state.btn2, x, y)) {
        state.log = "btn2 pressed!";
    }
    ui.textinput.is_pressed(state.input, x, y);
    ui.checkbox.is_pressed(state.cb, x, y);

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

    // slider
    if (ui.slider.is_pressed(state.slider, x, y)) {
        state.log = "slider: " + Math.floor(ui.slider.get_value(state.slider) + 0.5);
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
    ui.button.set_pressed(state.btn, false);
    ui.button.set_pressed(state.btn2, false);
    ui.slider.set_pressed(state.slider, false);
    state.popup_dragging = false;
};

joy.mousemoved = function(x, y, dx, dy)
{
    ui.button.is_hovered(state.btn, x, y);
    ui.button.is_hovered(state.btn2, x, y);
    ui.textinput.is_hovered(state.input, x, y);
    ui.slider.is_hovered(state.slider, x, y);
    ui.slider.drag_move(state.slider, x, y);
    if (state.popup_dragging) {
        ui.popup.drag_move(state.popup, x, y);
    }
    // tooltip 跟随鼠标
    ui.tooltip.set_position(state.tip, x + 12, y - 10);
    if (ui.button.is_hovered(state.btn, x, y)) {
        state.tip_text = "Button 1";
    } else if (ui.button.is_hovered(state.btn2, x, y)) {
        state.tip_text = "Button 2";
    } else if (ui.slider.is_hovered(state.slider, x, y)) {
        state.tip_text = "Slider: " + Math.floor(ui.slider.get_value(state.slider) + 0.5);
    } else if (ui.textinput.is_hovered(state.input, x, y)) {
        state.tip_text = "Text Input";
    } else {
        state.tip_text = "";
    }
};

joy.textinput = function(text)
{
    ui.textinput.insert(state.input, text);
};

joy.keypressed = function(key, scancode, repeat_, ctrl, shift, alt)
{
    if (key === "escape") {
        event.quit();
    } else if (key == "backspace") {
        ui.textinput.backspace(state.input);
    } else if (key == "delete") {
        ui.textinput.delete(state.input);
    } else if (key == "left") {
        if (shift) ui.textinput.select_left(state.input);
        else ui.textinput.cursor_left(state.input);
    } else if (key == "right") {
        if (shift) ui.textinput.select_right(state.input);
        else ui.textinput.cursor_right(state.input);
    } else if (key == "home") {
        if (shift) ui.textinput.cursor_home(state.input);
        else ui.textinput.cursor_home(state.input);
    } else if (key == "end") {
        if (shift) ui.textinput.cursor_end(state.input);
        else ui.textinput.cursor_end(state.input);
    } else if (key == "return") {
        if (ui.textinput.is_focused(state.input)) {
            state.log = "submit: " + ui.textinput.get_text(state.input);
        }
    } else if (key == "p") {
        state.popup_visible = !state.popup_visible;
        state.log = state.popup_visible ? "popup shown" : "popup hidden";
    } else if (ctrl) {
        if (key == "a") {
            ui.textinput.select_all(state.input);
        } else if (key == "v") {
            ui.textinput.paste(state.input);
        } else if (key == "c") {
            ui.textinput.copy(state.input);
        } else if (key == "x") {
            ui.textinput.cut(state.input);
        }
    }
};

joy.quit = function()
{
    if (ren) { r2d.destroy(ren); ren = null; }
    if (win) { window.destroy(win); win = null; }
};

// 场景系统 + UI 控件 + 查找测试（同步 Lua 功能）
var win, ren, font;
var mgr, menu_scene, menu_root_node;
var btn_start, btn_quit;
var game_btn_back = null;
var search_result = "";

joy.load = function() {
    win = get_env("sc_win");
    ren = get_env("sc_ren");
    mgr = get_env("sc_mgr");
    if (!mgr) {
        win = window.create("scene-demo-js", 800, 600, 0);
        ren = r2d.create(win);
        font = r2d.create_font(ren, "joy_beat/fonts/simhei.ttf", 24);

    var s = scene;
    mgr = s.mgr_create();

    // 菜单场景（JS label.create 不传文本，跳过）
    menu_scene = s.scene_create("menu");

    btn_start = ui.button.create(ren, 200, 50);
    ui.button.set_position(btn_start, 300, 220);
    ui.button.set_text(btn_start, font, "开始游戏", {r:255,g:255,b:255,a:255});

    btn_quit = ui.button.create(ren, 200, 50);
    ui.button.set_position(btn_quit, 300, 300);
    ui.button.set_text(btn_quit, font, "退出", {r:255,g:255,b:255,a:255});

    // 节点树（带名称和标签，用于查找测试）
    menu_root_node = s.node_create();
    s.node_set_name(menu_root_node, "tree_root");
    s.node_set_tag(menu_root_node, "root");
    s.node_set_position(menu_root_node, 400, 460);
    var c1 = s.node_create(); s.node_set_position(c1, -50, 50); s.node_add_child(menu_root_node, c1);
    s.node_set_name(c1, "left_branch"); s.node_set_tag(c1, "branch");
    var c2 = s.node_create(); s.node_set_position(c2, 50, 50);  s.node_add_child(menu_root_node, c2);
    s.node_set_name(c2, "right_branch"); s.node_set_tag(c2, "branch");
    s.scene_add_root(menu_scene, menu_root_node);

    s.mgr_push(mgr, menu_scene);
    }  // end if !mgr
};

joy.hotfix = function() {
    if (win) set_env("sc_win", win);
    if (ren) set_env("sc_ren", ren);
    if (mgr) set_env("sc_mgr", mgr);
};

function draw_node(n, ox, oy) {
    if (!n) return;
    var s = scene;
    var pos = s.node_get_position_v2(n);
    var wx = ox + pos.x, wy = oy + pos.y;
    if (ox !== 0 || oy !== 0) {
        r2d.set_color(ren, 80, 80, 120, 180);
        r2d.line(ren, ox, oy, wx, wy);
    }
    r2d.set_color(ren, 120, 200, 255, 220);
    r2d.circle(ren, "fill", wx, wy, 6, 10);
    var child = s.node_get_child(n);
    while (child) { draw_node(child, wx, wy); child = s.node_get_sibling(child); }
}

joy.draw = function() {
    r2d.begin(ren, 40, 45, 60, 255);
    var s = scene;
    var cur = s.mgr_current(mgr);
    var name = cur ? s.scene_get_name(cur) : "none";

    if (name === "menu") {
        ui.button.draw(btn_start);
        ui.button.draw(btn_quit);
        if (menu_root_node) draw_node(menu_root_node, 400, 460);
    } else if (name === "game") {
        r2d.set_color(ren, 100, 200, 255, 255);
        r2d.circle(ren, "fill", 400, 300, 60, 32);
        r2d.set_color(ren, 255, 255, 255, 255);
        r2d.print(ren, font, "Game Scene", 340, 280, {r:255,g:255,b:255,a:255});
        if (game_btn_back) ui.button.draw(game_btn_back);
    }

    r2d.set_color(ren, 200, 200, 200, 255);
    r2d.print(ren, font, "scene: " + name, 10, 550, {r:200,g:200,b:200,a:255});
    if (search_result.length > 0) {
        r2d.print(ren, font, search_result, 10, 515, {r:255,g:255,b:100,a:255});
    }
    r2d.end_frame(ren);
};

joy.mousepressed = function(x, y, button) {
    var cur = scene.mgr_current(mgr);
    var name = cur ? scene.scene_get_name(cur) : "";

    if (name === "menu") {
        if (ui.button.is_pressed(btn_start, x, y)) {
            ui.button.set_pressed(btn_start, false);
            console.log("-> game");
            var s = scene;
            var gs = s.scene_create("game");
            game_btn_back = ui.button.create(ren, 120, 40);
            ui.button.set_position(game_btn_back, 20, 20);
            ui.button.set_text(game_btn_back, font, "返回", {r:255,g:255,b:255,a:255});
            s.scene_add_root(gs, s.node_create());
            s.mgr_push(mgr, gs);
        } else if (ui.button.is_pressed(btn_quit, x, y)) {
            ui.button.set_pressed(btn_quit, false);
            event.quit();
        }
    } else if (name === "game") {
        if (game_btn_back && ui.button.is_pressed(game_btn_back, x, y)) {
            ui.button.set_pressed(game_btn_back, false);
            console.log("-> menu");
            game_btn_back = null;
            scene.mgr_pop(mgr);
        }
    }
};

joy.keypressed = function(key) {
    if (key === "escape" || key === "q") { event.quit(); return; }

    var cur = scene.mgr_current(mgr);
    var cur_name = cur ? scene.scene_get_name(cur) : "";
    if (cur_name !== "menu") return;  // 以下查找测试仅在菜单场景有效

    var s = scene;
    if (key === "1") {
        var found = s.scene_find_by_name(cur, "left_branch");
        if (found) {
            var pos = s.node_get_position_v2(found);
            var tag = s.node_get_tag(found);
            search_result = "find[left_branch] pos=(" + Math.floor(pos.x) + "," + Math.floor(pos.y) + ") tag=" + tag;
        } else {
            search_result = "find[left_branch] = null";
        }
    } else if (key === "2") {
        var found = s.scene_find_by_name(cur, "ghost");
        search_result = "find[ghost] = " + (found ? "ptr" : "null");
    } else if (key === "3") {
        var nodes = s.scene_find_by_tag(cur, "branch");
        var msg = "tag[branch] count=" + nodes.length;
        for (var i = 0; i < nodes.length; i++) {
            msg += "  " + (i+1) + "=" + s.node_get_name(nodes[i]);
        }
        search_result = msg;
    } else if (key === "4") {
        var found = s.mgr_find_node(mgr, "tree_root");
        if (found) {
            search_result = "mgr_find[tree_root] tag=" + found.tag + " children=" + found.children;
        } else {
            search_result = "mgr_find[tree_root] = null";
        }
    }
};
